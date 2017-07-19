#include "mythread.h"
#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/time.h>

#include <QDebug>
#include <QTime>

// 每块缓冲区的大小
#define REGION_SIZE       0x00040000
// DMA控制偏移量 0x00100000
#define CONFIG_DMA_OFFSET 0x00100000

MyThread::MyThread() :
    QThread()
{
    qsrand(QTime(0, 0, 0).secsTo(QTime::currentTime()));
    m_flag = false;

    pthread_mutex_init(&draw_thread_mutex , NULL);

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
    m_dma_mark = m_dma_data + CONFIG_DMA_OFFSET;
}

void MyThread::draw_a_scan()
{
	static int index = 0;
    int i;
    int _nCounter;
    const int pointQty = 512;
    QByteArray data;

    if (*((int *)m_dma_mark)) {
        pthread_mutex_lock(&draw_thread_mutex);
        *((int *)m_dma_mark) = 0;
        _nCounter = ((int *)m_dma_mark)[1];
        i = (_nCounter + 3) & 0x00000003;
        if (i == ((int *)m_dma_mark)[2]) {
            i = (i + 3) & 0x00000003;
        }
        ((int *)m_dma_mark)[2] = i;

        m_beam_data = m_dma_data + REGION_SIZE * i;
        data = QByteArray((const char *)(m_beam_data + index), pointQty);
        emit draw_data(data);
        pthread_mutex_unlock(&draw_thread_mutex);
		index ++;
		if (512 <= index) {
			index = 0;
		}
    }
}

void MyThread::run()
{
    ((int *)m_dma_mark)[4] = 128;

    while (1) {
        draw_a_scan();
        usleep(10000);
    }
}

MyThread::~MyThread()
{
}
