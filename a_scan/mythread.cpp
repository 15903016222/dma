#include "mythread.h"
#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <QDebug>
#include <QTime>

MyThread::MyThread() :
    QThread()
{
    qsrand(QTime(0, 0, 0).secsTo(QTime::currentTime()));
    m_flag = false;

    m_fd = open("/dev/mem", O_RDWR);
    if (m_fd < 0) {
        qDebug("open mem error");
    }

    m_dma_data = (unsigned int)mmap(0, 16 * 1024 * 1024,
                                    PROT_READ | PROT_WRITE, MAP_SHARED,
                                    m_fd, 0x2f000000);
    if (0 == m_dma_data) {
        qDebug("mmap error");
    }
    m_dma_mark = m_dma_data + 0x00100000; // DMA控制偏移
}

void MyThread::run()
{
    int i = 1;
    const int pointQty = 100;

    QByteArray data;
    while(m_flag) {
//        qDebug("i = %d ... ...", i);
        for (int j = 0; j < pointQty; j++){
            data.append(qrand());
        }
        emit draw_data(data);
        usleep(1000);
        i += 10;
        data.clear();
    }
}

MyThread::~MyThread()
{
}
