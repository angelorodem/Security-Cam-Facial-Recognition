#include "registrausuario.h"

RegistraUsuario::RegistraUsuario(std::shared_ptr<ChineseCluster> cluster_, QString url_) {
    cluster = cluster_;
    url = url_;
    nm = new QNetworkAccessManager;
}

void RegistraUsuario::envia_whatsapp(QString nome, QString numero) {
    qDebug() << "enviando" ;
    ;
    QString mensagem;
    mensagem = "Olá, " + nome + "!!!";
    QByteArray sendData = QByteArray::fromStdString(QString("servico=whatsapp&identificadorContato=55"
                          + numero  +
                          "&idMensagem=0&nomeContato=" + nome + "&mensagem=" + mensagem).toStdString());
    url = "http://" + url + "/";
    //qDebug() << url.toStdString() ;
    QUrl urle(url);
    //qDebug() << urle.toString().toStdString() ;
    //qDebug() << sendData.toStdString() ;
    QNetworkRequest request;
    request.setUrl(urle);
    request.setHeader(QNetworkRequest::ContentTypeHeader,
                      QVariant("application/x-www-form-urlencoded"));
    nm->post(request, sendData);
}


void RegistraUsuario::registra_novo_usuario(cv::Mat &foto, QString nome, QString numero,
        anet_type &net, dlib::frontal_face_detector &detector, dlib::shape_predictor &sp) {
    learn_from_photo(foto, nome, net, detector, sp);
    //envia_whatsapp(nome, numero);
}

std::vector<dlib::matrix<dlib::rgb_pixel>> RegistraUsuario::jitter_image(
dlib::matrix<dlib::rgb_pixel> &img) {
    thread_local dlib::rand rnd;
    std::vector<dlib::matrix<dlib::rgb_pixel>> crops;

    for (int i = 0; i < 100; ++i) {
        crops.push_back(dlib::jitter_image(img, rnd));
    }

    return crops;
}

void RegistraUsuario::salvar_nova_pessoa(cv::Mat &foto, std::string nome) {
    if (nome.empty()) {
        return;
    }

    cv::imwrite(QString(QDir::toNativeSeparators(QDir::currentPath()) +
                        QDir::toNativeSeparators("/knolege/")).toStdString() + nome +
                ".png", foto);
}

void RegistraUsuario::learn_from_photo(cv::Mat &foto, QString nome,
                                       anet_type &net,
                                       dlib::frontal_face_detector &detector,
                                       dlib::shape_predictor &sp) {
    qDebug() << "Treinando apartir de Foto" ;
    dlib::matrix<dlib::rgb_pixel> img;

    if (!foto.data) {
        qDebug() << "Erro!" ;
        emit erro_grave();
        return;
    }

    cvtColor(foto, foto, cv::COLOR_BGR2RGB);
    dlib::assign_image(img, dlib::cv_image<dlib::rgb_pixel>(foto));
    const uint32_t c_y = foto.rows / 2;
    const uint32_t c_x = foto.cols / 2;
    int32_t best_dist = 999999;
    dlib::matrix<dlib::rgb_pixel> face_chip_res;
    bool empt = true;

    for (auto face : detector(img)) {
        dlib::full_object_detection shape = sp(img, face);
        dlib::drectangle rect = shape.get_rect();
        empt = false;
        int32_t x = abs(rect.left() + (rect.width()  / 2) - c_x);
        int32_t y = abs(rect.top()  + (rect.height() / 2) - c_y);
        int32_t dist = x + y; //manhattan distance

        if (dist < best_dist) {
            dlib::matrix<dlib::rgb_pixel> face_chip;
            extract_image_chip(img, get_face_chip_details(shape, 150, 0.25), face_chip);
            face_chip_res = std::move(face_chip);
            best_dist = dist;
        }
    }

    cv::Mat rgb_mat;
    //cv::cvtColor(foto, rgb_mat, cv::COLOR_BGR2RGB);
    QPixmap show = QPixmap::fromImage(QImage((unsigned char *) rgb_mat.data,
                                      rgb_mat.cols, rgb_mat.rows, QImage::Format_RGB888).scaled(QSize(300, 300)));
    emit send_pic(show);

    if (empt) {
        emit nao_add_sem_pessoa();
        return;
    }

    std::shared_ptr<dlib::matrix<float, 0, 1>> face_d =
            std::make_shared<dlib::matrix<float, 0, 1>>();
    *face_d = dlib::mean(dlib::mat(net(jitter_image(face_chip_res))));
    cluster->adicionar_pessoa(face_d, nome.toStdString());
    qDebug() << "Treinado por foto: " << nome;
    cv::Mat colorc;
    cvtColor(toMat(face_chip_res), colorc, cv::COLOR_RGB2BGR);
    salvar_nova_pessoa(colorc, nome.toStdString());
    emit success();
}
