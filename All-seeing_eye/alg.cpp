#include "alg.h"

std::mutex Alg::alg_mutex;

void Alg::start_op() {
    emit send_op_thread(new std::thread([ = ] {exec();}));
}

void Alg::view_switch() {
    view_windows = !view_windows; // this can be made one line but im lazy now

    if (!view_windows) {
        img_show->hide();
    } else {
        img_show->show();
    }
}

void Alg::error_camera() {
    emit send_status("Erro\?");
}

void Alg::delete_camera() {
    close.store(true);
}

void Alg::set_intervalo(int interval) {
    reconize_fps = interval;
}

void Alg::set_tracking(bool track) {
    enable_tracking = track;
}

void Alg::set_use_color(bool use) {
    use_color = use;
}

void Alg::set_use_enhance(bool use) {
    enable_enhance = use;
}

void Alg::set_show_detections(bool use) {
    show_detections = use;

    if (!show_detections) {
        detect_show->hide();
    } else {
        detect_show->show();
    }
}

void Alg::set_enable_skip(bool use) {
    enable_skip = use;
}

Alg::Alg(uint32_t this_alg) {
    this_alg_id = this_alg;
    Qt::WindowFlags flags = 0;
    flags = Qt::Widget;
    img_show = new QMainWindow(this, flags);
    detect_show = new QMainWindow(this, flags);
    lab_frame = new QLabel(this);
    lab_frame->setPixmap(show_frame);
    lab_frame->show();
    lab_detect = new QLabel(this);
    lab_detect->setPixmap(show_detect);
    lab_detect->show();
    img_show->setCentralWidget(lab_frame);
    detect_show->setCentralWidget(lab_detect);
    srand(time(0));
    //clahe = cv::cuda::createCLAHE(3, cv::Size(8, 8));
    Gaussfilter = cv::cuda::createGaussianFilter(
                      CV_8UC3, CV_8UC3, cv::Size(ksize, ksize), sigma, sigma);
}

void Alg::parametros(std::string ip_, std::shared_ptr<ChineseCluster> cluster_,
                     std::string nome) {
    ip = ip_;
    cluster = cluster_;
    nome_camera = nome;
}

void Alg::keyPressEvent(QKeyEvent *e) {
    QWidget::keyPressEvent(e);
    /*if (e->key() == Qt::Key_Escape) {
        cluster->add_mode.store(!cluster->add_mode.load());
        qDebug() << "mode change!" ;
        e->accept();
    }*/
}

void Alg::exec() {
    if (ip.empty()) {
        emit close_camera(this_alg_id);
        return;
    }

    if (cluster == nullptr) {
        emit close_camera(this_alg_id);
        return;
    }

    if (close.load()) {
        emit close_camera(this_alg_id);
        return;
    }

    //std::shared_ptr<cv::Mat> frame;
    cv::Mat frame;
    //Detector facial
    frontal_face_detector detector = get_frontal_face_detector();
    //Alinhador de faces
    shape_predictor sp;
    deserialize(QString(QDir::toNativeSeparators(QDir::tempPath() +
                        "/shape.dat")).toStdString())
            >> sp;
    //Rede neural encoding facial 128D
    anet_type net;
    deserialize(QString(QDir::toNativeSeparators(QDir::tempPath() +
                        "/face.dat")).toStdString())
            >> net;
    //dlib::matrix<rgb_pixel> img;
    dlib::cv_image<rgb_pixel> img;
    dlib::cv_image<uint8_t> gry;
    int32_t counter = reconize_fps;
    std::vector<dlib::correlation_tracker> trackers_face;
    std::shared_ptr<std::vector<ChineseCluster::Analise>> resultado_clustering =
                std::make_shared<std::vector<ChineseCluster::Analise>>();
    //popula totemcluster e nomes com dados salvos
    //Camera cam;
    ///std::thread t = cam.run(ip);
    //emit send_status("Conectado");
    cv::VideoCapture video(ip);

    if (!video.isOpened()) {
        qDebug() << "Erro ao conectar com servidor" ;
        return;
    }

    while (true) {
        old_detection_handling();

        if (close.load()) {
            break;
        }

        if (!video.read(frame)) {
            qDebug() << "falha na leitura do video" ;
            return;
        }

        /*try {
            frame_ptr = cam.get_last();

            if ((*frame_ptr).cols == 10 && (*frame_ptr).rows == 10) {
                break;
            }
        } catch (std::exception &e) {
            qDebug() << QString::fromUtf8(e.what());
            break;
        }*/
        //http://www.antillia.com/sol9.2.0/4.22.html
        counter--;

        if (counter > 0) {
            if (view_windows) {
                if (resultado_clustering->size() <= 0) {
                    //mostra frame basico
                    if (skip_frame) {
                        cv::Mat rgb_mat;
                        cv::cvtColor(frame, rgb_mat, cv::COLOR_BGR2RGB);
                        show_frame = QPixmap::fromImage(QImage((unsigned char *) rgb_mat.data,
                                                               rgb_mat.cols, rgb_mat.rows, QImage::Format_RGB888).scaledToHeight(300));
                        lab_frame->setPixmap(show_frame);
                        img_show->update();
                        lab_frame->update();
                    }

                    skip_frame = enable_skip == true ? !skip_frame : true;
                    continue;
                }

                if (enable_tracking) {
                    cv::Mat gray;
                    cv::cvtColor(frame, gray, COLOR_BGR2GRAY);
                    gry = dlib::cv_image<uint8_t>(gray);

                    for (uint32_t i = 0; i < trackers_face.size(); ++i) {
                        trackers_face[i].update(gry); //----
                        face_window(frame, trackers_face, resultado_clustering.get(), i);
                    }
                }

                //mostra frame com os quadradinhos
                cv::Mat rgb_mat;
                cv::cvtColor(frame, rgb_mat, cv::COLOR_BGR2RGB);
                show_frame = QPixmap::fromImage(QImage((unsigned char *) rgb_mat.data, rgb_mat.cols, rgb_mat.rows,
                                                       QImage::Format_RGB888).scaledToHeight(300));
                lab_frame->setPixmap(show_frame);
                img_show->update();
                lab_frame->update();
                continue;
            }

            continue;
        }

        counter = reconize_fps;
        trackers_face.clear();
        std::vector<matrix<rgb_pixel>> faces;

        if (enable_enhance) {
            cv::cuda::Stream stream;
            cv::cuda::GpuMat gpuframe(frame);
            melhora_imagem(gpuframe, stream);

            if (!use_color) {
                cv::cuda::GpuMat gpu_gray;
                cv::cuda::cvtColor(gpuframe, gpu_gray, COLOR_BGR2GRAY, 0, stream);
                Mat gray;
                gpu_gray.download(gray);
                gry = dlib::cv_image<uint8_t>(gray);
            }

            stream.waitForCompletion();
            gpuframe.download(frame);
        }

        img = dlib::cv_image<rgb_pixel>(frame);

        if (!use_color && !enable_enhance) {
            cv::Mat ycr;
            cv::cvtColor(frame, ycr, COLOR_BGR2YCrCb);
            std::vector<cv::Mat> channels;
            cv::split(ycr, channels);
            Mat gray;
            //dlib::assign_image(gry, dlib::cv_image<uint8_t>(gray));
            gry = dlib::cv_image<uint8_t>(gray);
        }

        find_faces(faces, trackers_face, detector, sp, img, gry);

        //Roda algoritmo para achar as faces e coloca um quadrado ao redor

        if (faces.size() == 0) {
            continue;
        }

        std::vector<dlib::matrix<float, 0, 1>> face_descriptors = net(faces);
        //
        resultado_clustering->clear();
        resultado_clustering = cluster->analisa_dados(face_descriptors);
        add_encodings(resultado_clustering.get(), faces);
        new_detection_handling(resultado_clustering.get());
    }

    img_show->hide();
    //cam.close.store(true);
    //t.join();
    emit close_camera(this_alg_id);
    remove_detections();
    return;
}


//matrix<float,0,1> face_descriptor = mean(mat(net(jitter_image(faces[0]))));
//cria matriz depois da face mexida dps faz media
std::vector<dlib::matrix<rgb_pixel>> Alg::jitter_image(dlib::matrix<rgb_pixel> &img) {
    thread_local dlib::rand rnd;
    std::vector<dlib::matrix<rgb_pixel>> crops;

    for (int i = 0; i < 100; ++i) {
        crops.push_back(dlib::jitter_image(img, rnd));
    }

    return crops;
}

void Alg::add_encodings(std::vector<ChineseCluster::Analise> *resultado_totem,
                        std::vector<matrix<rgb_pixel>> &faces) {
    for (uint32_t var = 0; var < resultado_totem->size(); ++var) {
        ChineseCluster::Analise &res = (*resultado_totem)[var];

        if (res.pessoa != nullptr) {  //TODO: CHECK
            if (res.pessoa->desconhecida) {
                bool ext = false;

                for (uint32_t chk_ext = 0; chk_ext < faces_unk_saved.size(); ++chk_ext) {
                    if (res.pessoa->unique_id == faces_unk_saved[chk_ext]) {
                        ext = true;
                        break;
                    }
                }

                if (ext) {
                    return;
                }

                Mat colorc = toMat(faces[var]);
                //cvtColor(, colorc, COLOR_RGB2BGR);
                faces_unk_saved.push_back(res.pessoa->unique_id);
                salvar_nova_pessoa_desconhecida(colorc, res.pessoa->nome);
            }
        }
    }
}

void Alg::find_faces(std::vector<matrix<rgb_pixel>> &faces,
                     std::vector<dlib::correlation_tracker> &trackers_face,
                     frontal_face_detector &detector,
                     shape_predictor &sp,
                     dlib::cv_image<rgb_pixel> &image,
                     dlib::cv_image<uint8_t> &gray) {
    if (use_color) {
        std::vector<dlib::rectangle> dets = detector(image);

        for (uint32_t i = 0; i < dets.size(); ++i) {
            dlib::rectangle &face = dets[i];
            auto shape = sp(image, face);
            matrix<rgb_pixel> face_chip;
            extract_image_chip(image, get_face_chip_details(shape, 150, 0.25), face_chip);
            faces.push_back(std::move(face_chip));

            if (view_windows && enable_tracking) {
                trackers_face.push_back(correlation_tracker()); //---
                trackers_face.back().start_track(image, drectangle(shape.get_rect())); //---
            }
        }
    } else {
        std::vector<dlib::rectangle> dets = detector(gray);

        for (uint32_t i = 0; i < dets.size(); ++i) {
            dlib::rectangle &face = dets[i];
            dlib::full_object_detection shape = sp(gray, face);
            matrix<rgb_pixel> face_chip;
            extract_image_chip(image, get_face_chip_details(shape, 150, 0.25), face_chip);
            faces.push_back(std::move(face_chip));

            if (view_windows && enable_tracking) {
                trackers_face.push_back(correlation_tracker()); //---
                trackers_face.back().start_track(gray, drectangle(shape.get_rect())); //---
            }
        }
    }

    if (show_detections && faces.size() > 0) {
        matrix<rgb_pixel> aae = tile_images(faces);
        cv::Mat rgb_mat = dlib::toMat(aae);
        cv::cvtColor(rgb_mat, rgb_mat, cv::COLOR_BGR2RGB);
        show_detect = QPixmap::fromImage(QImage((unsigned char *) rgb_mat.data,
                                                rgb_mat.cols, rgb_mat.rows, QImage::Format_RGB888));
        detect_show->show();
        lab_detect->setPixmap(show_detect);
        detect_show->update();
        lab_detect->update();
    } else {
        detect_show->hide();
    }
}

void Alg::face_window(Mat &frame,
                      const std::vector<dlib::correlation_tracker> &trackers_face,
                      std::vector<ChineseCluster::Analise> *resultado_totem,
                      const uint32_t i) {
    drectangle drec = trackers_face[i].get_position();
    cv::Rect rct;

    if (drec.left() + drec.width() >= frame.cols) {
        rct.x = frame.cols - drec.width();
    } else if (drec.left() < 1) {
        rct.x = 1;
    } else {
        rct.x = drec.left();
    }

    if (drec.top() + drec.height() >= frame.rows) {
        rct.y = frame.rows - drec.height();
    } else if (drec.top() < 1) {
        rct.y = 1;
    } else {
        rct.y = drec.top();
    }

    rct.height = drec.height();
    rct.width = drec.width();
    int baselin = 0;

    if (!((*resultado_totem)[i].pessoa == nullptr)) { //TODO: CHECK
        Size sz = getTextSize((*resultado_totem)[i].pessoa->nome,
                              FONT_HERSHEY_COMPLEX_SMALL, 1.2, 2, &baselin);
        cv::putText(frame, (*resultado_totem)[i].pessoa->nome,
                    cvPoint(drec.left() + drec.width() / 2 - sz.width / 2,
                            drec.top() - 10),
                    FONT_HERSHEY_COMPLEX_SMALL,
                    1.2, cv::Scalar(0, 200, 0), 2, LINE_4);
    }

    cv::rectangle(frame, rct, Scalar(0, 255, 0), 2, LINE_4);
}



void Alg::salvar_nova_pessoa(cv::Mat &foto, std::string nome) {
    if (nome.empty()) {
        return;
    }

    imwrite(QString(QDir::toNativeSeparators(QDir::currentPath()) +
                    QDir::toNativeSeparators("/knolege/")).toStdString() + nome +
            ".png", foto);
}

void Alg::salvar_nova_pessoa_desconhecida(Mat &foto, std::string nome) {
    if (nome.empty()) {
        return;
    }

    imwrite(QString(QDir::toNativeSeparators(QDir::currentPath()) +
                    QDir::toNativeSeparators("/desconhecidas/")).toStdString() + nome
            + " " + std::to_string(detect_counter) + ".png", foto);
    detect_counter++;
}

void Alg::old_detection_handling() {
    time_t now = time(NULL);

    for (uint32_t detect_c = 0; detect_c < send_detect_vector.size(); ++detect_c) {
        //se passou 4 secs que foi detectado
        if ((now - send_detect_vector[detect_c].second) > 6) {
            emit send_not_detected(send_detect_vector[detect_c].first);
            send_detect_vector.erase(send_detect_vector.begin() + detect_c);
        }
    }
}

void Alg::new_detection_handling(std::vector<ChineseCluster::Analise> *resultado_totem) {
    //detectados agora
    for (uint32_t results_c = 0; results_c < resultado_totem->size(); ++results_c) {
        ChineseCluster::Analise &who_detect = (*resultado_totem)[results_c];
        bool isInDetectionList = false;

        if (who_detect.pessoa == nullptr) { //TODO: CHECK
            continue;
        }

        //já detectados
        for (uint32_t detect_c = 0; detect_c < send_detect_vector.size(); ++detect_c) {
            //se já estava detectado e foi novamente
            if (who_detect.pessoa->unique_id == send_detect_vector[detect_c].first) {
                send_detect_vector[detect_c].second = time(NULL);
                isInDetectionList = true;
                break;
            }
        }

        //se não foi detectado antes e foi agora
        if (isInDetectionList == false) {
            send_detect_vector.push_back(std::make_pair(who_detect.pessoa->unique_id,
                                         time(NULL)));
            emit send_detection(QString::fromStdString(nome_camera),
                                QString::fromStdString(who_detect.pessoa->nome),
                                (quint32)who_detect.pessoa->unique_id);
        }
    }
}

void Alg::remove_detections() {
    for (uint32_t detect_c = 0; detect_c < send_detect_vector.size(); ++detect_c) {
        emit send_not_detected(send_detect_vector[detect_c].first);
        send_detect_vector.erase(send_detect_vector.begin() + detect_c);
    }
}

void Alg::melhora_imagem(cv::cuda::GpuMat &frame, cv::cuda::Stream &stream) {
    //remove ruidos da imagem
    cv::cuda::fastNlMeansDenoisingColored(frame, frame, 3, 3, 7, 21, stream);
    //contraste
    cv::cuda::GpuMat lab_mat;
    cv::cuda::cvtColor(frame, lab_mat, CV_BGR2YCrCb, 0, stream);
    std::vector<cv::cuda::GpuMat> channels;
    cv::cuda::split(lab_mat, channels, stream);
    //clahe->apply(channels[0], channels[0]);
    cv::cuda::equalizeHist(channels[0], channels[0], stream);
    cv::cuda::merge(channels, lab_mat, stream);
    cv::cuda::cvtColor(lab_mat, frame, CV_YCrCb2BGR, 0, stream);
    //sharpen
    cv::cuda::GpuMat imagem_borrada(frame.size(), frame.type());
    Gaussfilter->apply(frame, imagem_borrada, stream);
    cv::cuda::addWeighted(frame, 1.5, imagem_borrada, -0.5, 0.0, imagem_borrada, -1, stream);
    frame = imagem_borrada;
    //
}

uint64_t Alg::getDetect_counter() const {
    return detect_counter;
}

void Alg::setDetect_counter(const uint64_t &value) {
    detect_counter = value;
}


