#pragma once

#include <opencv2/core.hpp>
#include <opencv2/opencv.hpp>
#include <opencv2/video.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/videoio.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/core/ocl.hpp>
#include <opencv2/core/cuda.hpp>
#include <opencv2/photo/cuda.hpp>

#include <dlib/dnn.h>
#include <dlib/clustering.h>
#include <dlib/string.h>
#include <dlib/image_io.h>
#include <dlib/dir_nav.h>
#include <dlib/image_processing.h>
#include <dlib/image_processing/frontal_face_detector.h>
#include <dlib/opencv.h>
#include <dlib/matrix.h>

#include <thread>
#include <mutex>
#include <shared_mutex>
#include <thread>
#include <utility>
#include <iostream>
#include <random>
#include <ctime>
#include <string>
#include <queue>
#include <cmath>
#include <vector>
#include <cstdint>
#include <tuple>
#include <atomic>
#include <memory>

#include <cryptopp/sha.h>

#include <QDir>
#include <QFile>
#include <QStringList>
#include <QString>
#include <QFileInfo>
#include <QMainWindow>
#include <QLabel>
#include <QPixmap>
#include <QKeyEvent>
#include <QObject>
#include <QDialog>
#include <QWidget>
#include <QApplication>
#include <QHBoxLayout>
#include <QPushButton>
#include <QVarLengthArray>
#include <QDebug>
#include <QListWidgetItem>
#include <QTime>
#include <QNetworkRequest>
#include <QNetworkAccessManager>
#include <QUrlQuery>
#include <QByteArray>
#include <QUrl>

#include "Chinese/pessoa.h"

template <template <int, template<typename>class, int, typename> class block, int N, template<typename>class BN, typename SUBNET>
using residual = dlib::add_prev1<block<N, BN, 1, dlib::tag1<SUBNET>>>;

template <template <int, template<typename>class, int, typename> class block, int N, template<typename>class BN, typename SUBNET>
using residual_down =
    dlib::add_prev2<dlib::avg_pool<2, 2, 2, 2, dlib::skip1<dlib::tag2<block<N, BN, 2, dlib::tag1<SUBNET>>>>>>;

template <int N, template <typename> class BN, int stride, typename SUBNET>
using block  =
    BN<dlib::con<N, 3, 3, 1, 1, dlib::relu<BN<dlib::con<N, 3, 3, stride, stride, SUBNET>>>>>;

template <int N, typename SUBNET> using ares      =
    dlib::relu<residual<block, N, dlib::affine, SUBNET>>;
template <int N, typename SUBNET> using ares_down =
    dlib::relu<residual_down<block, N, dlib::affine, SUBNET>>;

template <typename SUBNET> using alevel0 = ares_down<256, SUBNET>;
template <typename SUBNET> using alevel1 =
    ares<256, ares<256, ares_down<256, SUBNET>>>;
template <typename SUBNET> using alevel2 =
    ares<128, ares<128, ares_down<128, SUBNET>>>;
template <typename SUBNET> using alevel3 =
    ares<64, ares<64, ares<64, ares_down<64, SUBNET>>>>;
template <typename SUBNET> using alevel4 = ares<32, ares<32, ares<32, SUBNET>>>;

using anet_type =
    dlib::loss_metric<dlib::fc_no_bias<128, dlib::avg_pool_everything<
    alevel0<
    alevel1<
    alevel2<
    alevel3<
    alevel4<
    dlib::max_pool<3, 3, 2, 2, dlib::relu<dlib::affine<dlib::con<32, 7, 7, 2, 2,
    dlib::input_rgb_image_sized<150>
    >>>>>>>>>>>>;


