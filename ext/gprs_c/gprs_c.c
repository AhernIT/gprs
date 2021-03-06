
#include <stdio.h>
#include <stdbool.h>
#include <assert.h>
#include <time.h>

#include <ruby.h>

#include "defs.h"
#include "gprs.h"
#include "report.h"

VALUE gprs = Qnil;

VALUE packet_type(VALUE self, VALUE data, VALUE print);
VALUE parse_report(VALUE self, VALUE data, VALUE print);
VALUE packet_process(VALUE self, VALUE data, VALUE print);

void Init_gprs_c()
{
  gprs = rb_define_module("GprsC");
  rb_define_singleton_method(gprs, "packet_type_c", packet_type, 2);
  rb_define_singleton_method(gprs, "parse_report_c", parse_report, 2);
  rb_define_singleton_method(gprs, "packet_process_c", packet_process, 2);
}

VALUE make_symbol(const char * name)
{
  return ID2SYM(rb_intern(name));
}

VALUE hash_from_report(report_t report)
{
  VALUE hash = rb_hash_new();
  struct tm time = report_stotm(report.time);
  char time_str[80];
  double latitude = report_stolat(report.lat, report.lat_south);
  double longitude = report_stolon(report.lon, report.lon_west);

  strftime(time_str, sizeof(time_str), "%Y-%m-%dT%H:%M:%S+00:00", &time);

  rb_hash_aset(hash, make_symbol("device_id"),    INT2NUM(report.device_id));
  rb_hash_aset(hash, make_symbol("time_secs"),    INT2NUM(report.time));
  rb_hash_aset(hash, make_symbol("time"),         rb_str_new2(time_str));
  if (report.has_gps) {
    rb_hash_aset(hash, make_symbol("gps_valid"),  INT2NUM(!report.gps_invalid));
    rb_hash_aset(hash, make_symbol("latitude"),   DBL2NUM(latitude));
    rb_hash_aset(hash, make_symbol("longitude"),  DBL2NUM(longitude));
    rb_hash_aset(hash, make_symbol("speed"),      INT2NUM(report.speed));
  }
  if (report.has_cog) {
    rb_hash_aset(hash, make_symbol("cog"),        INT2NUM(report.cog));
  }
  rb_hash_aset(hash, make_symbol("report_type"),  INT2NUM(report.type));
  if (report_has_code(report.type)) {
    rb_hash_aset(hash, make_symbol("code"),       INT2NUM(report.code));
  }
  if (report.has_modsts) {
    rb_hash_aset(hash, make_symbol("modsts"),     INT2NUM(report.modsts));
  }
  if (report.has_temp) {
    rb_hash_aset(hash, make_symbol("temperature"),INT2NUM(report.temp));
  }
  if (report.has_cell) {
    rb_hash_aset(hash, make_symbol("cell_id"),    INT2NUM(report.cell_id));
    rb_hash_aset(hash, make_symbol("signal_csq"), INT2NUM(report.signal));
  }
  if (report.has_lac) {
    rb_hash_aset(hash, make_symbol("lac"),        INT2NUM(report.lac));
  }
  rb_hash_aset(hash, make_symbol("input_1"),      INT2NUM(report.input_1));
  rb_hash_aset(hash, make_symbol("input_2"),      INT2NUM(report.input_2));
  rb_hash_aset(hash, make_symbol("output_1"),     INT2NUM(report.output_1));
  rb_hash_aset(hash, make_symbol("output_2"),     INT2NUM(report.output_2));

  switch (report.type) {
  case REPORT_TYPE_EXTENDED_DATA:
  {
    VALUE ext    = rb_hash_new();

    switch (report.ext_type) {
    case REPORT_DATA_TYPE_ADDITIONAL_IO:
    {
      additional_io_t * addio = &report.ext.additional_io;
      if (addio->has_int_voltage) {
        double int_voltage = report_cvtov(addio->int_voltage);
        rb_hash_aset(ext, make_symbol("int_voltage"),     DBL2NUM(int_voltage));
      }
      if (addio->has_ext_voltage) {
        double ext_voltage = report_cvtov(addio->ext_voltage);
        rb_hash_aset(ext, make_symbol("ext_voltage"),     DBL2NUM(ext_voltage));
      }
      if (addio->has_adc_input_1) {
        rb_hash_aset(ext, make_symbol("analog_input_1"),  INT2NUM(addio->adc_input_1));
      }
      if (addio->has_adc_input_2) {
        rb_hash_aset(ext, make_symbol("analog_input_2"),  INT2NUM(addio->adc_input_2));
      }
      if (addio->has_input_3) {
        rb_hash_aset(ext, make_symbol("input_3"),         INT2NUM(addio->input_3));
      }
      if (addio->has_output_3) {
        rb_hash_aset(ext, make_symbol("output_3"),        INT2NUM(addio->output_3));
      }
      if (addio->has_orientation) {
        rb_hash_aset(ext, make_symbol("orientation"),     INT2NUM(addio->orientation));
      }
      break;
    }
    case REPORT_DATA_TYPE_FC_TXN_COMPLETE:
    {
      fc_txn_t * txn = &report.ext.fc_txn;
      rb_hash_aset(ext, make_symbol("fc_txn_id"),         INT2NUM(txn->txn_id));
      rb_hash_aset(ext, make_symbol("fc_ticks"),          INT2NUM(txn->ticks));
      break;
    }
    default:
      // Do nothing
      break;
    }

    rb_hash_aset(hash, make_symbol("ext_type"),           INT2NUM(report.ext_type));
    rb_hash_aset(hash, make_symbol("ext"),                ext);

    break;
  }
  default:
    // Do nothing;
    break;
  }

  return hash;
}

VALUE hash_from_report_raw(report_t report)
{
  VALUE hash = rb_hash_new();

  rb_hash_aset(hash, make_symbol("ref"),          INT2NUM(report.ref));
  rb_hash_aset(hash, make_symbol("has_cell"),     INT2NUM(report.has_cell));
  rb_hash_aset(hash, make_symbol("has_gps"),      INT2NUM(report.has_gps));
  rb_hash_aset(hash, make_symbol("report_type"),  INT2NUM(report.type));
  rb_hash_aset(hash, make_symbol("has_modsts"),   INT2NUM(report.has_modsts));
  rb_hash_aset(hash, make_symbol("has_temp"),     INT2NUM(report.has_temp));
  rb_hash_aset(hash, make_symbol("id_len"),       INT2NUM(report.id_len));
  rb_hash_aset(hash, make_symbol("lat_south"),    INT2NUM(report.lat_south));
  rb_hash_aset(hash, make_symbol("lon_west"),     INT2NUM(report.lon_west));
  rb_hash_aset(hash, make_symbol("has_cog"),      INT2NUM(report.has_cog));
  rb_hash_aset(hash, make_symbol("has_lac"),      INT2NUM(report.has_lac));
  rb_hash_aset(hash, make_symbol("time_secs"),    INT2NUM(report.time));
  rb_hash_aset(hash, make_symbol("device_id"),    INT2NUM(report.device_id));
  rb_hash_aset(hash, make_symbol("lat"),          INT2NUM(report.lat));
  rb_hash_aset(hash, make_symbol("gps_invalid"),  INT2NUM(report.gps_invalid));
  rb_hash_aset(hash, make_symbol("lon"),          INT2NUM(report.lon));
  rb_hash_aset(hash, make_symbol("speed"),        INT2NUM(report.speed));
  rb_hash_aset(hash, make_symbol("cog"),          INT2NUM(report.cog));
  rb_hash_aset(hash, make_symbol("code"),         INT2NUM(report.code));
  rb_hash_aset(hash, make_symbol("modsts"),       INT2NUM(report.modsts));
  rb_hash_aset(hash, make_symbol("temp"),         INT2NUM(report.temp));
  rb_hash_aset(hash, make_symbol("cell_id"),      INT2NUM(report.cell_id));
  rb_hash_aset(hash, make_symbol("signal_csq"),   INT2NUM(report.signal));
  rb_hash_aset(hash, make_symbol("lac"),          INT2NUM(report.lac));
  rb_hash_aset(hash, make_symbol("input_1"),      INT2NUM(report.input_1));
  rb_hash_aset(hash, make_symbol("input_2"),      INT2NUM(report.input_2));
  rb_hash_aset(hash, make_symbol("output_1"),     INT2NUM(report.output_1));
  rb_hash_aset(hash, make_symbol("output_2"),     INT2NUM(report.output_2));

  switch (report.type) {
  case REPORT_TYPE_EXTENDED_DATA:
  {
    VALUE ext = rb_hash_new();

    switch (report.ext_type) {
    case REPORT_DATA_TYPE_ADDITIONAL_IO:
    {
      additional_io_t * addio = &report.ext.additional_io;
      rb_hash_aset(ext, make_symbol("has_int_voltage"), INT2NUM(addio->has_int_voltage));
      rb_hash_aset(ext, make_symbol("has_ext_voltage"), INT2NUM(addio->has_ext_voltage));
      rb_hash_aset(ext, make_symbol("has_adc_input_1"), INT2NUM(addio->has_adc_input_1));
      rb_hash_aset(ext, make_symbol("has_adc_input_2"), INT2NUM(addio->has_adc_input_2));
      rb_hash_aset(ext, make_symbol("has_input_3"),     INT2NUM(addio->has_input_3));
      rb_hash_aset(ext, make_symbol("has_output_3"),    INT2NUM(addio->has_output_3));
      rb_hash_aset(ext, make_symbol("has_orientation"), INT2NUM(addio->has_orientation));
      rb_hash_aset(ext, make_symbol("int_voltage"),     INT2NUM(addio->int_voltage));
      rb_hash_aset(ext, make_symbol("ext_voltage"),     INT2NUM(addio->ext_voltage));
      rb_hash_aset(ext, make_symbol("adc_input_1"),     INT2NUM(addio->adc_input_1));
      rb_hash_aset(ext, make_symbol("adc_input_2"),     INT2NUM(addio->adc_input_2));
      rb_hash_aset(ext, make_symbol("input_3"),         INT2NUM(addio->input_3));
      rb_hash_aset(ext, make_symbol("output_3"),        INT2NUM(addio->output_3));
      rb_hash_aset(ext, make_symbol("orientation"),     INT2NUM(addio->orientation));
      break;
    }
    case REPORT_DATA_TYPE_FC_TXN_COMPLETE:
    {
      fc_txn_t * txn = &report.ext.fc_txn;
      rb_hash_aset(ext, make_symbol("fc_txn_id"),       INT2NUM(txn->txn_id));
      rb_hash_aset(ext, make_symbol("fc_ticks"),        INT2NUM(txn->ticks));
      break;
    }
    default:
      break;
    }

    rb_hash_aset(hash, make_symbol("ext_type"),         INT2NUM(report.ext_type));
    rb_hash_aset(hash, make_symbol("ext"),              ext);

    break;
  }
  default:
    // Do nothing
    break;
  }


  return hash;
}

int data_to_packet(VALUE data, uint8_t * packet)
{
  int i;
  int size = (int)RARRAY_LEN(data);

  // Convert Ruby array to uint8_t
  for (i = 0; i < size; i++) {
    packet[i] = NUM2UINT(rb_ary_entry(data, i));
  }

  return size;
}

VALUE packet_to_data(uint8_t * packet, int size)
{
  VALUE data = rb_ary_new();

  // Convert uint8_t array to Ruby array
  for (int i = 0; i < size; i++) {
    rb_ary_push(data, UINT2NUM(packet[i]));
  }

  return data;
}

VALUE packet_type(VALUE self, VALUE data, VALUE print)
{
  uint8_t packet[GPRS_PACKET_MAX_SIZE];
  int size = data_to_packet(data, packet);
  bool verbose = (print == Qtrue);
  int rc, type = GPRS_PACKET_UNKNOWN;

  // Size limit
  if (size > GPRS_PACKET_MAX_SIZE) size = GPRS_PACKET_MAX_SIZE;

  rc = gprs_preprocess(packet, &size, verbose);
  if (rc == GPRS_RC_SUCCESS) {
    type = gprs_packet_type(packet, size);
  } else {
    if (verbose) printf("Invalid GPRS packet! Error: %d\n", rc);
  }

  return INT2NUM(type);
}

VALUE parse_report(VALUE self, VALUE data, VALUE print)
{
  uint8_t packet[GPRS_PACKET_MAX_SIZE];
  int size = data_to_packet(data, packet);
  bool verbose = (print == Qtrue);
  int i, rc, count;
  VALUE results = rb_ary_new();

  // Size limit
  if (size > GPRS_PACKET_MAX_SIZE) size = GPRS_PACKET_MAX_SIZE;

  // Preprocess
  rc = gprs_preprocess(packet, &size, verbose);
  if (rc == GPRS_RC_SUCCESS) {
    int type = gprs_packet_type(packet, size);

    if (type == GPRS_PACKET_REPORT) {
      report_t reports[10];

      // Parse report
      count = report_parse(packet, size, reports);

      for (i = 0; i < count; i++) {
        if (verbose) {
          report_print(reports[i]);
        }

        // Create hash for report
        VALUE result = hash_from_report(reports[i]);

        // Add to array
        rb_ary_push(results, result);
      }
    } else {
      if (verbose) printf("Don't know how to parse packet of type %d\n", type);
    }
  } else {
    if (verbose) printf("Invalid GPRS packet! Error: %d\n", rc);
  }

  return results;
}

VALUE packet_process(VALUE self, VALUE data, VALUE print)
{
  uint8_t packet[GPRS_PACKET_MAX_SIZE];
  int size = data_to_packet(data, packet);
  bool verbose = (print == Qtrue);
  int rc;

  VALUE processedData;

  // Size limit
  if (size > GPRS_PACKET_MAX_SIZE) size = GPRS_PACKET_MAX_SIZE;

  rc = gprs_preprocess(packet, &size, verbose);
  if (rc == GPRS_RC_SUCCESS) {
    processedData = packet_to_data(packet, size);
  } else {
    if (verbose) printf("Invalid GPRS packet! Error: %d\n", rc);
  }

  return processedData;
}
