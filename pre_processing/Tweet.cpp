#include "Tweet.h"

namespace PreProcessing::TweetStream {
	time_duration::sec_type TweetStreamProcess::ToTimeDuration(std::string& time_str_format) {
		ptime current_time = time_from_string(time_str_format);
		time_duration duration = current_time - start_time;
		return duration.total_seconds();
	}
}