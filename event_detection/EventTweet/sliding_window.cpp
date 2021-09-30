#include "event_detection/EventTweet/sliding_window.h"

using EventTweet::SlidingWindow::SnapShot;
using EventTweet::SlidingWindow::WordTweetPair;

namespace EventTweet::SlidingWindow {

	void SlidingWindow::Slide(SnapShot&& snapshot) {
		if (sliding_window.size() >= WINDOW_SIZE) {
			sliding_window.pop_front();
		}

		sliding_window.emplace_back(std::move(snapshot));

		return;
	}
}