#include "camera.h"

Camera::Camera() {
}

std::thread Camera::run(std::string ip) {
    return std::thread([ = ] {start_cap(ip);});
}

std::shared_ptr<cv::Mat> Camera::get_last() {
    std::unique_lock<std::mutex> lock(camera_mutex); //only one pass

    if (isEmpty_buffer.load()) { //is empty?
        lock.unlock();         //than release the lock and wait

        while (isEmpty_buffer.load()) { //while empty we wait
        }

        lock.lock(); //if its not empty anymore we try to lock again
    }

    std::shared_ptr<cv::Mat> ret = buffer.back();
    buffer.pop();

    if (buffer.empty()) {
        isEmpty_buffer.store(true);
    }

    return ret;
}

void Camera::flag_close() {
    close.store(true);
}

void Camera::start_cap(std::string ip) {
    cv::VideoCapture video(ip);
    std::shared_ptr<cv::Mat> frame;

    try {
        if (!video.isOpened()) {
            qDebug() << "Erro ao conectar com servidor" ;
            frame = std::make_shared<cv::Mat>();
            (*frame) = cv::Mat(10, 10, CV_8UC3);
            close.store(true);
            add_frame(frame);
            return;
        }

        while (true) {
            if (close.load()) {
                return;
            }

            frame = std::make_shared<cv::Mat>();

            if (!video.read((*frame))) {
                qDebug() << "falha na leitura do video" ;
                (*frame) = cv::Mat(10, 10, CV_8UC3);
                close.store(true);
            }

            if ((*frame).empty()) {
                (*frame) = cv::Mat(10, 10, CV_8UC3);
                close.store(true);
            }

            add_frame(frame);
        }
    } catch (std::exception &e) {
        std::cout << " EXPT: " << e.what() << std::endl;
        (*frame) = cv::Mat(10, 10, CV_8UC3);
        close.store(true);
        add_frame(frame);
    }
}

void Camera::add_frame(std::shared_ptr<cv::Mat> &frame) {
    std::lock_guard<std::mutex> lock(camera_mutex);

    if (buffer.size() < max_buffer_size) {
        buffer.push(frame);
        isEmpty_buffer.store(false);
    } else {
        qDebug() << "Perdendo frames! " << count_lost ;
        count_lost++;
        //sadly if full we cant take more stream data,
        //and stuff is lost, wich is not ideal
    }
}
