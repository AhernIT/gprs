
require "gprs/defs"
require "gprs/report"
require "gprs/version"

# C bindings
require "gprs_c"

module Gprs
  def self.parse_report(packet)
    # Preprocess GPRS packet first
    valid, packet = Gprs::Defs.preprocess(packet)

    report = nil
    if valid
      # Parse report
      report = Report.read(packet.pack("C*"))
    else
      puts "Invalid report packet!"
    end
  end
end

module GprsC
  # Nothing for now
end