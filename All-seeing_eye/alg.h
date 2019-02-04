#pragma once

#include "imports.h"
#include "Chinese/chinesecluster.h"

//#include "camera.h"

#include <cryptopp/sha.h>

using namespace cv;
using namespace dlib;

class Alg : public QWidget {
    Q_OBJECT

  public slots:
    void start_op();
    void view_switch();
    void error_camera();
    void delete_camera();
    void set_intervalo(int interval);
    void set_tracking(bool track);
    void set_use_color(bool use);
    void set_use_enhance(bool use);
    void set_show_detections(bool use);
    void set_enable_skip(bool use);
  signals:
    void send_op_thread(std::thread *thrd);
    void send_status(QString stats);
    void send_detection(QString nome_cam, QString pessoa, quint32 id);
    void send_not_detected(quint32 id);
    void close_camera(quint32 alg_id);
  public:
    explicit Alg(uint32_t this_alg);
    void parametros(std::string ip_, std::shared_ptr<ChineseCluster> cluster_,
                    std::string nome);
    virtual ~Alg() {}

    uint32_t this_alg_id = 0;
    uint64_t getDetect_counter() const;
    void setDetect_counter(const uint64_t &value);

  private:
    static std::mutex alg_mutex;

    std::string nome_camera;

    std::atomic<bool> close = {false};

    void keyPressEvent(QKeyEvent *e);

    std::vector<dlib::matrix<rgb_pixel>> jitter_image(dlib::matrix<rgb_pixel> &img);
    void exec();

    void add_encodings(std::vector<ChineseCluster::Analise> *
                       resultado_totem,
                       std::vector<matrix<rgb_pixel>> &faces);

    void find_faces(std::vector<matrix<rgb_pixel>> &faces,
                    std::vector<dlib::correlation_tracker> &trackers_face,
                    frontal_face_detector &detector,
                    shape_predictor &sp,
                    dlib::cv_image<rgb_pixel> &image,
                    dlib::cv_image<uint8_t> &gray);

    void face_window(cv::Mat &frame,
                     const std::vector<correlation_tracker> &trackers_face,
                     std::vector<ChineseCluster::Analise> *resultado_totem,
                     const uint32_t i);

    void salvar_nova_pessoa(cv::Mat &foto, std::string nome);
    void salvar_nova_pessoa_desconhecida(cv::Mat &foto, std::string nome);

    void old_detection_handling();
    void new_detection_handling(
        std::vector<ChineseCluster::Analise> *resultado_totem);
    void remove_detections();

    void melhora_imagem(cv::cuda::GpuMat &frame, cv::cuda::Stream &stream);

    int reconize_fps = 12;

    std::string ip = "";
    //TotemCluster *cluster = nullptr;
    std::shared_ptr<ChineseCluster> cluster;
    bool view_windows = false;

    QMainWindow *img_show;
    QMainWindow *detect_show;
    QLabel *lab_frame;
    QLabel *lab_detect;
    QPixmap show_frame;
    QPixmap show_detect;


    uint64_t detect_counter = 0;
    bool enable_skip = true;
    bool skip_frame = false;
    bool enable_tracking = false;
    bool use_color = true;
    bool enable_enhance = true;
    bool show_detections = false;

    std::vector<std::pair<uint32_t, time_t>> send_detect_vector;
    std::vector<uint32_t> faces_unk_saved;

    //cv::Ptr<cv::cuda::CascadeClassifier> face_detector_cv;
    //cv::Ptr<cv::cuda::CLAHE> clahe;
    cv::Ptr<cv::cuda::Filter> Gaussfilter;

    int ksize = 3;
    int sigma = 13;


};


