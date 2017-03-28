/*
 * camera_c_p.hpp
 *
 *  Created on: Jan 29, 2016
 *      Author: Nafis Zaman
 */

#ifndef _CAMERA_C_P_HPP_
#define _CAMERA_C_P_HPP_

#include "wallaby/camera.hpp"

namespace Private
{
  class DeviceSingleton
  {
  public:    
#ifndef EMSCRIPTEN
    static ::Camera::Device *instance();
#endif

  private:
 #ifndef EMSCRIPTEN
    static ::Camera::Device *s_device;
 #endif
  };
}

#endif