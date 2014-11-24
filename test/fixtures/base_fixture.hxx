

#ifndef _LWAPTK_BASE_FIXTURE_H_
#define _LWAPTK_BASE_FIXTURE_H_

#include <iostream>

#include <gtest/gtest.h>


class BaseFixture : public testing::Test {
 protected:

  virtual void SetUp() {
  }
  
};


#endif