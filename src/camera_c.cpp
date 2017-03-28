/*
 * camera_c.cpp
 *
 *  Created on: Jan 29, 2016
 *      Author: Nafis Zaman
 */

#include "wallaby/camera.h"
#include "wallaby/camera.hpp"
#include "warn.hpp"
#include "nyi.h"
#include "camera_c_p.hpp"

#include <iostream>
#include <cstdlib>

using namespace Private;

int camera_open()
{  
  return camera_open_at_res(LOW_RES);
}

int camera_open_black()
{
	return camera_open_device_model_at_res(0, BLACK_2017, LOW_RES);
}

int camera_open_at_res(enum Resolution res)
{
  if(res != LOW_RES) {
    WARN("only LOW_RES is currently supported");
    return 0;
  }
  
  return camera_open_device(0, res);
}

int camera_open_device(int number, enum Resolution res)
{
  int ret = camera_open_device_model_at_res(number, WHITE_2016, res);
  return ret;
}

int camera_open_device_model_at_res(int number, enum Model model, enum Resolution res)
{
#ifndef EMSCRIPTEN
  bool ret = DeviceSingleton::instance()->open(number, res, model);
  if(!ret) return 0;

  return 1;
#else
  return 0;
#endif
}

int camera_load_config(const char *name)
{
#ifndef EMSCRIPTEN
  Config *config = Config::load(Camera::ConfigPath::path(name));
  if(!config) return 0;
  DeviceSingleton::instance()->setConfig(*config);
  delete config;
  return 1;
#else
  return 0;
#endif
}

void set_camera_width(int width)
{
  if(width <= 0) {
    std::cout << "Camera width must be greater than 0." << std::endl;
    return;
  }
#ifndef EMSCRIPTEN
  DeviceSingleton::instance()->setWidth(width);
#endif
}

void set_camera_height(int height)
{
  if(height <= 0) {
    std::cout << "Camera height must be greater than 0." << std::endl;
    return;
  }
#ifndef EMSCRIPTEN
  DeviceSingleton::instance()->setHeight(height);
#endif
}

int get_camera_width(void)
{
#ifndef EMSCRIPTEN
  return DeviceSingleton::instance()->width();
#else
  return -1;
#endif
}

int get_camera_height(void)
{
#ifndef EMSCRIPTEN
  return DeviceSingleton::instance()->height();
#else
  return -1;
#endif
}

int camera_update(void)
{
#ifndef EMSCRIPTEN
  return DeviceSingleton::instance()->update() ? 1 : 0;
#else
  return 0;
#endif
}

pixel get_camera_pixel(point2 p)
{
#ifndef EMSCRIPTEN
  const cv::Mat &mat = DeviceSingleton::instance()->rawImage();
  if(mat.empty()) {
    WARN("camera image is empty");
    return pixel();
  }
  
  if(p.x < 0 || p.y < 0 || p.x >= mat.cols || p.y >= mat.rows) {
    WARN("point isn't within the image");
    return pixel();
  }
  
  const cv::Vec3b v = mat.at<cv::Vec3b>(p.y, p.x);
  
  pixel ret;
  ret.r = v[2];
  ret.g = v[1];
  ret.b = v[0];
  
  return ret;
#else
  pixel ret; // TODO: better error value?
  return ret;
#endif
}

int get_channel_count(void)
{
#ifndef EMSCRIPTEN
  return DeviceSingleton::instance()->channels().size();
#else
  return -1;
#endif
}

bool check_channel(int i)
{
#ifndef EMSCRIPTEN
  const Camera::ChannelPtrVector &channels = DeviceSingleton::instance()->channels();
  if(i < 0 || i >= channels.size()) {
    std::cout << "Channel must be in the range 0 .. " << (channels.size() - 1) << std::endl;
    return false;
  }
  return true;
#else
  return false;
#endif
}

bool check_channel_and_object(int i, int j)
{
#ifndef EMSCRIPTEN
  const Camera::ChannelPtrVector &channels = DeviceSingleton::instance()->channels();
  if(i < 0 || i >= channels.size()) {
    if(!channels.size()) std::cout << "Active configuration doesn't have any channels.";
    else std::cout << "Channel must be in the range 0 .. " << (channels.size() - 1);
    std::cout << std::endl;
    return false;
  }
  const Camera::ObjectVector *objs = channels[i]->objects();
  if(j < 0 || j >= objs->size()) {
    std::cout << "No such object " << j << std::endl;
    return false;
  }
  return true;
#else
  return true;
#endif
}

int get_object_count(int channel)
{
#ifndef EMSCRIPTEN
  if(!check_channel(channel)) return -1;
  return DeviceSingleton::instance()->channels()[channel]->objects()->size();
#else
  return -1;
#endif
}

double get_object_confidence(int channel, int object)
{
#ifndef EMSCRIPTEN
  if(!check_channel_and_object(channel, object)) return 0.0;
  const Camera::Object &o = (*DeviceSingleton::instance()->channels()[channel]->objects())[object];
  return o.confidence();
#else
  return 0.0;
#endif
}


const char *get_object_data(int channel, int object)
{
#ifndef EMSCRIPTEN
  if(!check_channel_and_object(channel, object)) return 0;
  const Camera::Object &o = (*DeviceSingleton::instance()->channels()[channel]->objects())[object];
  return o.data();
#else
  return 0;
#endif
}

int get_code_num(int channel, int object)
{
#ifndef EMSCRIPTEN
  if(!check_channel_and_object(channel, object)) return -1;
  const char *data = get_object_data(channel, object);
  if(!data) return 0;
  return atoi(data);
#else
  return -1;
#endif
}

int get_object_data_length(int channel, int object)
{
#ifndef EMSCRIPTEN
  if(!check_channel_and_object(channel, object)) return 0;
  const Camera::Object &o = (*DeviceSingleton::instance()->channels()[channel]->objects())[object];
  return o.dataLength();
#else
  return 0;
#endif
}

int get_object_area(int channel, int object)
{
#ifndef EMSCRIPTEN
  if(!check_channel_and_object(channel, object)) return -1;
  const Camera::Object &o = (*DeviceSingleton::instance()->channels()[channel]->objects())[object];
  return o.boundingBox().area();
#else
  return -1;
#endif
}

rectangle get_object_bbox(int channel, int object)
{
#ifndef EMSCRIPTEN
  if(!check_channel_and_object(channel, object)) return create_rectangle(-1, -1, 0, 0);
  const Camera::Object &o = (*DeviceSingleton::instance()->channels()[channel]->objects())[object];
  return o.boundingBox().toCRectangle();
#else
  return create_rectangle(-1, -1, 0, 0);
#endif
}

int get_object_bbox_ulx(int channel, int object)
{
  return get_object_bbox(channel, object).ulx;
}

int get_object_bbox_uly(int channel, int object)
{
  return get_object_bbox(channel, object).uly;
}

int get_object_bbox_brx(int channel, int object)
{
  const rectangle r = get_object_bbox(channel, object);
  return r.ulx + r.width;
}

int get_object_bbox_bry(int channel, int object)
{
  const rectangle r = get_object_bbox(channel, object);
  return r.uly + r.height;
}

int get_object_bbox_width(int channel, int object)
{
  return get_object_bbox(channel, object).width;
}

int get_object_bbox_height(int channel, int object)
{
  return get_object_bbox(channel, object).height;
}

point2 get_object_centroid(int channel, int object)
{
#ifndef EMSCRIPTEN
  if(!check_channel_and_object(channel, object)) return create_point2(-1, -1);
  const Camera::Object &o = (*DeviceSingleton::instance()->channels()[channel]->objects())[object];
  return o.centroid().toCPoint2();
#else
  return create_point2(-1, -1);
#endif
}

int get_object_centroid_column(int channel, int object)
{
  return get_object_centroid(channel, object).x;
}

int get_object_centroid_x(int channel, int object)
{
  return get_object_centroid(channel, object).x;
}

int get_object_centroid_row(int channel, int object)
{
  return get_object_centroid(channel, object).y;
}

int get_object_centroid_y(int channel, int object)
{
  return get_object_centroid(channel, object).y;
}

point2 get_object_center(int channel, int object)
{
#ifndef EMSCRIPTEN
  if(!check_channel_and_object(channel, object)) return create_point2(-1, -1);
  const Camera::Object &o = (*DeviceSingleton::instance()->channels()[channel]->objects())[object];
  return o.boundingBox().center().toCPoint2();
#else
  return create_point2(-1, -1);
#endif
}

int get_object_center_column(int channel, int object)
{
  return get_object_center(channel, object).x;
}

int get_object_center_x(int channel, int object)
{
  return get_object_center(channel, object).x;
}

int get_object_center_row(int channel, int object)
{
  return get_object_center(channel, object).y;
}

int get_object_center_y(int channel, int object)
{
  return get_object_center(channel, object).y;
}

void camera_close()
{
#ifndef EMSCRIPTEN
  DeviceSingleton::instance()->close();
#endif
}

void set_camera_config_base_path(const char *const path)
{
#ifndef EMSCRIPTEN
  Camera::ConfigPath::setBasePath(path);
#endif
}

const unsigned char *get_camera_frame_row(unsigned row)
{
#ifndef EMSCRIPTEN
  return DeviceSingleton::instance()->rawImage().ptr(row);
#else
  return nullptr;
#endif
}

const unsigned char *get_camera_frame()
{
#ifndef EMSCRIPTEN
  return DeviceSingleton::instance()->bgr();
#else
  return nullptr;
#endif
}

unsigned get_camera_element_size()
{
#ifndef EMSCRIPTEN
  return DeviceSingleton::instance()->rawImage().elemSize();
#else
  return 0;
#endif
}
