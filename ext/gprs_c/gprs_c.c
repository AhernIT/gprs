
#include <stdio.h>
#include <stdbool.h>
#include <assert.h>

#include <ruby.h>

#include "gprs.h"
#include "report.h"

VALUE gprs = Qnil;

VALUE parse_report(VALUE self, VALUE data, VALUE print);

void Init_gprs_c()
{
  gprs = rb_define_module("GprsC");
  rb_define_singleton_method(gprs, "parse_report", parse_report, 2);
}

VALUE make_symbol(const char * name)
{
  return ID2SYM(rb_intern(name));
}

VALUE hash_from_report(report_t report)
{
  VALUE hash = rb_hash_new();

  rb_hash_aset(hash, make_symbol("ref"),          INT2NUM(report.ref));
  rb_hash_aset(hash, make_symbol("has_cell"),     INT2NUM(report.has_cell));
  rb_hash_aset(hash, make_symbol("has_gps"),      INT2NUM(report.has_gps));
  rb_hash_aset(hash, make_symbol("report_type"),  INT2NUM(report.type));
  rb_hash_aset(hash, make_symbol("has_modsts"),   INT2NUM(report.has_modsts));
  rb_hash_aset(hash, make_symbol("has_temp"),     INT2NUM(report.has_temp));
  rb_hash_aset(hash, make_symbol("id_len"),       INT2NUM(report.id_len));
  rb_hash_aset(hash, make_symbol("input_1"),      INT2NUM(report.input_1));
  rb_hash_aset(hash, make_symbol("input_2"),      INT2NUM(report.input_2));
  rb_hash_aset(hash, make_symbol("output_1"),     INT2NUM(report.output_1));
  rb_hash_aset(hash, make_symbol("output_2"),     INT2NUM(report.output_2));
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
  rb_hash_aset(hash, make_symbol("signal"),       INT2NUM(report.signal));
  rb_hash_aset(hash, make_symbol("lac"),          INT2NUM(report.lac));

  switch (report.type) {
  case REPORT_TYPE_EXTENDED_DATA:
  {
    VALUE ext_data = rb_hash_new();
    VALUE fields = rb_hash_new();

    switch (report.data_type) {
    case REPORT_DATA_TYPE_ADDITIONAL_IO:
    {
      additional_io_t * addio = &report.data.additional_io;
      rb_hash_aset(fields, make_symbol("has_int_voltage"),  INT2NUM(addio->has_int_voltage));
      rb_hash_aset(fields, make_symbol("has_ext_voltage"),  INT2NUM(addio->has_ext_voltage));
      rb_hash_aset(fields, make_symbol("has_adc_input_1"),  INT2NUM(addio->has_adc_input_1));
      rb_hash_aset(fields, make_symbol("has_adc_input_2"),  INT2NUM(addio->has_adc_input_2));
      rb_hash_aset(fields, make_symbol("has_input_3"),      INT2NUM(addio->has_input_3));
      rb_hash_aset(fields, make_symbol("has_output_3"),     INT2NUM(addio->has_output_3));
      rb_hash_aset(fields, make_symbol("has_orientation"),  INT2NUM(addio->has_orientation));
      rb_hash_aset(fields, make_symbol("int_voltage"),      INT2NUM(addio->int_voltage));
      rb_hash_aset(fields, make_symbol("ext_voltage"),      INT2NUM(addio->ext_voltage));
      rb_hash_aset(fields, make_symbol("adc_input_1"),      INT2NUM(addio->adc_input_1));
      rb_hash_aset(fields, make_symbol("adc_input_2"),      INT2NUM(addio->adc_input_2));
      rb_hash_aset(fields, make_symbol("input_3"),          INT2NUM(addio->input_3));
      rb_hash_aset(fields, make_symbol("output_3"),         INT2NUM(addio->output_3));
      rb_hash_aset(fields, make_symbol("orientation"),      INT2NUM(addio->orientation));
      break;
    }
    default:
      break;
    }

    rb_hash_aset(ext_data, make_symbol("data_type"),        INT2NUM(report.data_type));
    rb_hash_aset(ext_data, make_symbol("data"),             fields);

    rb_hash_aset(hash, make_symbol("ext_data"),             ext_data);

    break;
  }
  default:
    // Do nothing
    break;
  }


  return hash;
}

VALUE parse_report(VALUE self, VALUE data, VALUE print)
{
  int size = RARRAY_LEN(data);
  uint8_t packet[size];
  report_t reports[10];
  int i, rc;
  VALUE result = rb_hash_new();

  // Convert Ruby array to uint8_t
  for (i = 0; i < size; i++) {
    packet[i] = NUM2UINT(rb_ary_entry(data, i));
  }

  // Preprocess
  rc = gprs_preprocess(packet, &size);
  if (rc == GPRS_RC_SUCCESS) {
    // Parse report
    report_parse(packet, size, reports);

    // Populate hash
    result = hash_from_report(reports[0]);
  } else {
    printf("Invalid report packet!\n");
  }

  return result;
}