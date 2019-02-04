#pragma once

#include "imports.h"
#include "pessoa.h"


class Ball {
  public:
    Ball(dlib::matrix<float, 0, 1> centro_, float raio_, std::shared_ptr<Pessoa> pess_) {
        centro = centro_;
        raio = raio_;
        pessoa = pess_;
        //qDebug() << "New ball: " << QString::number(raio_) << " " << QString::fromStdString(pessoa->nome);
    }

    std::shared_ptr<Pessoa> pessoa;
    dlib::matrix<float, 0, 1> centro;
    float raio;
};
