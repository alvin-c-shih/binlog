#include <binlog/Time.hpp>

namespace binlog {

std::chrono::nanoseconds ticksToNanoseconds(std::uint64_t frequency, std::int64_t ticks)
{
  return std::chrono::nanoseconds{ticks * std::nano::den / std::int64_t(frequency)};
}

std::chrono::nanoseconds clockToNsSinceEpoch(const ClockSync& clockSync, std::uint64_t clockValue)
{
  using nanos = std::chrono::nanoseconds;

  const std::int64_t diffValue = std::int64_t(clockValue - clockSync.clockValue);
  const nanos diff = ticksToNanoseconds(clockSync.clockFrequency, diffValue);
  const nanos sinceEpoch = nanos{clockSync.nsSinceEpoch} + diff;

  return sinceEpoch;
}

void nsSinceEpochToBrokenDownTimeUTC(std::chrono::nanoseconds sinceEpoch, BrokenDownTime& dst)
{
  using clock = std::chrono::system_clock;

  // assumption: system_clock measures Unix Time
  // (i.e., time since 1970.01.01 00:00:00 UTC, not counting leap seconds).
  // Valid since C++20, tested by unit tests.
  const clock::time_point tp{sinceEpoch};
  const std::time_t tt = clock::to_time_t(tp);

  // TODO(benedek) platform: use gmtime_r/gmtime_s where available
  const std::tm* result = std::gmtime(&tt);
  if (result == nullptr) { return; }

  static_cast<std::tm&>(dst) = *result;

  // set the sub-second part
  const auto seconds = std::chrono::duration_cast<std::chrono::seconds>(sinceEpoch);
  const std::chrono::nanoseconds remainder{sinceEpoch - seconds};

  dst.tm_nsec = int(remainder.count());
}

} // namespace binlog