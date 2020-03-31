/*
 * Copyright (c) 2020 <copyright holder> <email>
 *
 * Permission is hereby granted, free of charge, to any person
 * obtaining a copy of this software and associated documentation
 * files (the "Software"), to deal in the Software without
 * restriction, including without limitation the rights to use,
 * copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following
 * conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
 * OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
 * HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 * OTHER DEALINGS IN THE SOFTWARE.
 */

#ifndef PRIMARYDISPLAY_H
#define PRIMARYDISPLAY_H

#include <QtCore/QtCore>
#include <QtWidgets/QtWidgets>
#include <QtQmqtt/QtQmqtt>

class PrimaryDisplay : public QFrame
{
    Q_OBJECT
public:
    PrimaryDisplay(QFrame *parent = 0);
    ~PrimaryDisplay();

protected:
    void showEvent(QShowEvent*);

public slots:
    void connected();
    void disconnected();
    void error(const QMQTT::ClientError error);
    void subscribed(const QString& topic, const quint8 qos);
    void unsubscribed(const QString& topic);
    void published(const quint16 msgid, const quint8 qos);
    void pingresp();
    void received(const QMQTT::Message& message);
    
private slots:
    void hideLightning();
    
private:
    void displayConditions(QByteArray);
    void displayLightningEvent(QByteArray);
    void dislplayNoisefloorEvent(QByteArray);
    void displayTunables(QByteArray);
    void displayStartup(QByteArray);
    
    QMQTT::Client *m_mqttClient;
    QGridLayout *m_layout;
    QLabel *m_temp;
    QLabel *m_humidity;
    QLabel *m_lightning;
    QLabel *m_noise;
    QLabel *m_threshold;
    QLabel *m_ssid;
    QLabel *m_appid;
};

#endif // PRIMARYDISPLAY_H
