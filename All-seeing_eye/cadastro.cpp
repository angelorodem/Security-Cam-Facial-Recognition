#include "cadastro.h"
#include "ui_cadastro.h"



Cadastro::Cadastro(QString ip_whats_p, QString ip_camera_p,
                   std::shared_ptr<ChineseCluster> _clstr) : ui(new Ui::Cadastro) {
    ui->setupUi(this);
    setWindowFlags(Qt::WindowTitleHint | Qt::WindowStaysOnTopHint);
    cluster = _clstr;
    ip_whats = ip_whats_p;
    ip_camera = ip_camera_p;
    device_n = -1;
    //Detector facial
    detector = dlib::get_frontal_face_detector();
    ui->error_label->hide();
    ui->error_label->setWordWrap(true);
    ui->succ->hide();
    //Alinhador de faces
    dlib::deserialize(QString(QDir::toNativeSeparators(QDir::tempPath() + "/shape.dat")).toStdString())
            >> sp;
    //Rede neural encoding facial 128D
    dlib::deserialize(QString(QDir::toNativeSeparators(QDir::tempPath() + "/face.dat")).toStdString())
            >> net;
}

Cadastro::Cadastro(QString ip_whats_p, int device_nbr, std::shared_ptr<ChineseCluster> _clstr):
    ui(new Ui::Cadastro) {
    ui->setupUi(this);
    setWindowFlags(Qt::WindowTitleHint | Qt::WindowStaysOnTopHint);
    cluster = _clstr;
    ip_whats = ip_whats_p;
    ip_camera = "";
    device_n = device_nbr;
    ui->succ->hide();
    //Detector facial
    detector = dlib::get_frontal_face_detector();
    ui->error_label->hide();
    ui->error_label->setWordWrap(true);
    //Alinhador de faces
    dlib::deserialize(QString(QDir::toNativeSeparators(QDir::tempPath() + "/shape.dat")).toStdString())
            >> sp;
    //Rede neural encoding facial 128D
    dlib::deserialize(QString(QDir::toNativeSeparators(QDir::tempPath() + "/face.dat")).toStdString())
            >> net;
}

Cadastro::~Cadastro() {
    delete ui;
}

void Cadastro::on_pronto_clicked() {
    ui->error_label->hide();
    ui->error_label->setText("");
    ui->succ->hide();
    QRegExp re("\\d*");  // a digit (\d), zero or more times (*)

    if (!re.exactMatch(ui->numero->text())) {
        ui->error_label->show();
        ui->error_label->setText("Numero não deve ter espaços ou traços, exemplo: 4999913399");
        return;
    }

    if (ui->numero->text().size() > 12) {
        ui->error_label->show();
        ui->error_label->setText("Numero do Whatsapp parece errado :D");
        return;
    }

    ui->pronto->setEnabled(false);
    RegistraUsuario *ru = new RegistraUsuario(cluster, ip_whats);
    connect(ru, SIGNAL(erro_grave()), this, SLOT(erro_grave()));
    connect(ru, SIGNAL(nao_add_invalido()), this, SLOT(nao_add_invalido()));
    connect(ru, SIGNAL(nao_add_sem_pessoa()), this, SLOT(nao_add_sem_pessoa()));
    connect(ru, SIGNAL(send_pic(QPixmap)), this, SLOT(recebe_pic(QPixmap)));
    connect(ru, SIGNAL(success()), this, SLOT(success()));
    cv::VideoCapture *cap;

    if (device_n >= 0) {
        cap = new cv::VideoCapture(device_n);
    } else {
        cap = new cv::VideoCapture(ip_camera.toStdString());
    }

    cv::Mat frame;
    int co = 0;

    for (int var = 0; var < 10; ++var) {
        if (!cap->read(frame)) {
            qDebug() << "falha na leitura do video" ;
            co++;
            delete cap;

            if (device_n >= 0) {
                cap = new cv::VideoCapture(device_n);
            } else {
                cap = new cv::VideoCapture(ip_camera.toStdString());
            }

            continue;
        }

        if (frame.empty()) {
            qDebug() << "Frame vaziu" ;
            co++;
            delete cap;

            if (device_n >= 0) {
                cap = new cv::VideoCapture(device_n);
            } else {
                cap = new cv::VideoCapture(ip_camera.toStdString());
            }

            continue;
        }
    }

    if (co >= 3) {
        qDebug() << "saindo, erro!" ;
        return;
    }

    ru->registra_novo_usuario(frame,
                              ui->nome->text(),
                              ui->numero->text(),
                              net, detector, sp);
    disconnect(ru, SIGNAL(erro_grave()), this, SLOT(erro_grave()));
    disconnect(ru, SIGNAL(nao_add_invalido()), this, SLOT(nao_add_invalido()));
    disconnect(ru, SIGNAL(nao_add_sem_pessoa()), this, SLOT(nao_add_sem_pessoa()));
    disconnect(ru, SIGNAL(success()), this,
               SLOT(success()));
    delete ru;
    delete cap;
    ui->pronto->setEnabled(true);
}

void Cadastro::nao_add_invalido() {
    ui->error_label->show();
    ui->error_label->setText("Dados inválidos");
}


void Cadastro::nao_add_sem_pessoa() {
    ui->error_label->show();
    ui->error_label->setText("Não consigo te ver, se posicione melhor em frente a camera, iremos tirar uma foto!");
}

void Cadastro::success() {
    ui->succ->setText("Feito! Obrigado.");
    ui->succ->show();
    ui->nome->clear();
    ui->numero->clear();
}

void Cadastro::erro_grave() {
    ui->error_label->show();
    ui->error_label->setText("Erro grave! culpa do evandro... tente novamente :D");
}

void Cadastro::recebe_pic(QPixmap imagem) {
    ui->logo->setPixmap(imagem);
    ui->logo->update();
}

void Cadastro::keyPressEvent(QKeyEvent *e) {
    if (e->key() == Qt::Key_F9) {
        this->close();
    }
}
