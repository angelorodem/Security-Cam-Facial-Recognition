#pragma once

#include "imports.h"

class Camera {
  signals:
    void error();
  public:
    Camera();
    std::thread run(std::string ip);
    std::shared_ptr<cv::Mat> get_last();
    void flag_close();
    std::atomic<bool> close = {false};

  private:
    uint64_t count_lost = 0;

    void start_cap(std::string ip);

    std::queue<std::shared_ptr<cv::Mat>> buffer;
    void add_frame(std::shared_ptr<cv::Mat> &frame);
    std::mutex camera_mutex;
    std::atomic<bool> isEmpty_buffer = {true}; //using 2 typing cases to mess with haters


    static constexpr uint32_t max_buffer_size = 68;

    uint64_t times = 0;
};


