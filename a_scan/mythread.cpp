#include "mythread.h"
#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/time.h>
#include <QDebug>
#include <QTime>

MyThread::MyThread() :
    QThread()
{
    m_dma = Dma::instance();
    m_flag = true;
}

void MyThread::draw_a_scan()
{
    QByteArray data;
    static int index = 0;
    const int pointQty = 512;
    const char *temp = NULL;

    if ((temp = m_dma->read_data()) == NULL) {
        return ;
    }

    data = QByteArray(temp + index, pointQty);
    emit draw_data(data);
    index++;
    if (index > (512 * 1024 - 512)) {
        index = 0;
    }
}

void MyThread::run()
{
    m_dma->set_frame_count (128);
    while (m_flag) {
        draw_a_scan();
        usleep(1000);
    }
}

MyThread::~MyThread()
{
}
