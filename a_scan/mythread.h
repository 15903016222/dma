#ifndef __MYTHREAD_H__
#define __MYTHREAD_H__

#include <QThread>
#include <QTimer>

class MyThread : public QThread
{
    Q_OBJECT
public:
    explicit MyThread();
    ~MyThread();
    void set_flag(bool flag){m_flag = flag;}

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
};

#endif // MYTHREAD_H
