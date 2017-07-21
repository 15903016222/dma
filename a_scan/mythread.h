#ifndef __MYTHREAD_H__
#define __MYTHREAD_H__

#include "dma.h"
#include <pthread.h>
#include <QThread>
#include <QTimer>

using namespace DplSource;

class MyThread : public QThread
{
    Q_OBJECT
public:
    explicit MyThread();
    ~MyThread();

public:
    void draw_a_scan();
    void set_flag (bool flag) {
        m_flag = flag;
    }

signals:
    void draw_data(QByteArray drawData);

protected:
    void run();

public:
    Dma *m_dma;
    bool m_flag;
};

#endif // MYTHREAD_H
