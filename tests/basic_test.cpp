#define BOOST_TEST_MODULE winhttp_test
#include <boost/test/unit_test.hpp>

#include "NpEndpoint.hpp"
#include "NpEventEngine.hpp"
#include "NpListener.hpp"

BOOST_AUTO_TEST_SUITE(gnp_test_suite)

BOOST_AUTO_TEST_CASE(Endpoint) {

  // gnp::NpEndpoint
  gnp::NpListener l;
}

BOOST_AUTO_TEST_CASE(Tasks) {

  gnp::NpEventEngine ev;
  // just run
  {
    std::atomic_bool b = false;
    absl::AnyInvocable<void()> closure = [&]() { b = true; };
    ev.Run(std::move(closure));
    std::this_thread::sleep_for(std::chrono::seconds(1));
    BOOST_CHECK_EQUAL(b, true);
  }

  // post
  {
    std::atomic_bool b = false;
    absl::AnyInvocable<void()> closure = [&]() { b = true; };
    ev.RunAfter(std::chrono::seconds(2), std::move(closure));
    std::this_thread::sleep_for(std::chrono::seconds(3));
    BOOST_CHECK_EQUAL(b, true);
  }

  // post and cancel
  {
    std::atomic_bool b = false;
    absl::AnyInvocable<void()> closure = [&]() { b = true; };
    auto handle = ev.RunAfter(std::chrono::seconds(2), std::move(closure));
    std::this_thread::sleep_for(std::chrono::seconds(1));
    ev.Cancel(handle);
    BOOST_CHECK_EQUAL(b, false);
  }
}

BOOST_AUTO_TEST_SUITE_END()