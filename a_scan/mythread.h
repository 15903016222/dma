#ifndef __MYTHREAD_H__
#define __MYTHREAD_H__

#include <pthread.h>
#include <QThread>
#include <QTimer>

class MyThread : public QThread
{
    Q_OBJECT
public:
    explicit MyThread();
    ~MyThread();

public:
    void set_flag(bool flag){m_flag = flag;}
    void draw_a_scan();

signals:
    void draw_data(QByteArray drawData);

protected:
    void run();

private:
    bool m_flag;
    int  m_fd;
    unsigned int m_dma_mark;
    unsigned int m_dma_data;
    unsigned int m_store_buffer;
    unsigned int m_beam_data;

    pthread_mutex_t draw_thread_mutex  ;
};

#endif // MYTHREAD_H
