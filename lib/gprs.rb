
require "gprs/version.rb"

require "gprs_c"
require "gprs_kaitai"

module Gprs
  # If it's a string with format "0xA 0xB 0xC..." convert to byte array
  def self.process_packet(packet)
    if packet.is_a? String and packet.include? "0x"
      packet.split(" ").map{ |x| x.gsub("0x", "").to_i(16) }
    else
      packet
    end
  end

  def self.packet_type(packet, log = false)
    packet = process_packet(packet)

    GprsC.packet_type_c(packet, log)
  end

  def self.parse_report(packet, log = false)
    packet = process_packet(packet)

    GprsC.parse_report_c(packet, log)
  end

  def self.parse_command(packet, log = false)
    # Convert to readable format
    packet = process_packet(packet)

    GprsKaitai.parse_command(packet, log)
  end
end
