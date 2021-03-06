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

#include "primarydisplay.h"

PrimaryDisplay::PrimaryDisplay(QFrame *parent) : QFrame(parent)
{
    m_humidity = new QLabel();
    m_temp = new QLabel();
    m_lightning = new QLabel();
    m_noise = new QLabel();
    m_dewpoint = new QLabel();
    m_ssid = new QLabel();
    m_appid = new QLabel();
    m_rssi = new QLabel();
    m_uptime = new QLabel();
    m_rainToday = new QLabel();
    m_rainTotal = new QLabel();
    
    QLabel *tlabel = new QLabel("Temperature");
    QLabel *hlabel = new QLabel("Humidity");
    
    m_layout = new QGridLayout();
    m_layout->addWidget(tlabel, 0, 0, Qt::AlignCenter);
    m_layout->addWidget(hlabel, 0, 1, Qt::AlignCenter);
    m_layout->addWidget(m_temp, 1, 0, 2, 1, Qt::AlignCenter);
    m_layout->addWidget(m_humidity, 1, 1, 2, 1, Qt::AlignCenter);
    m_layout->addWidget(m_lightning, 3, 0, 1, 2, Qt::AlignCenter);
    m_layout->addWidget(m_rainToday, 4, 0, Qt::AlignLeft);
    m_layout->addWidget(m_rainTotal, 4, 1, Qt::AlignLeft);
    m_layout->addWidget(m_noise, 5, 0, Qt::AlignLeft);
    m_layout->addWidget(m_dewpoint, 5, 1, Qt::AlignLeft);
    m_layout->addWidget(m_ssid, 6, 0, Qt::AlignLeft);
    m_layout->addWidget(m_appid, 6, 1, Qt::AlignLeft);
    m_layout->addWidget(m_rssi, 7, 0, Qt::AlignLeft);
    m_layout->addWidget(m_uptime, 7, 1, Qt::AlignLeft);
    m_layout->setContentsMargins(0, 0, 0, 0);
    m_layout->setSpacing(0);
    setLayout(m_layout);
    
    m_mqttClient = new QMQTT::Client("172.24.1.13", 1883, false, false);
    m_mqttClient->setClientId(QHostInfo::localHostName());
    m_mqttClient->connectToHost();
    m_mqttClient->setAutoReconnect(true);
    m_mqttClient->setAutoReconnectInterval(10000);
    
    connect(m_mqttClient, SIGNAL(connected()), this, SLOT(connected()));
    connect(m_mqttClient, SIGNAL(disconnected()), this, SLOT(disconnected()));
    connect(m_mqttClient, SIGNAL(error(const QMQTT::ClientError)), this, SLOT(error(const QMQTT::ClientError)));

    connect(m_mqttClient, SIGNAL(subscribed(const QString&, const quint8)), this, SLOT(subscribed(const QString&, const quint8)));
    connect(m_mqttClient, SIGNAL(unsubscribed(const QString&)), this, SLOT(unsubscribed(const QString&)));
    connect(m_mqttClient, SIGNAL(pingresp()), this, SLOT(pingresp()));
    connect(m_mqttClient, SIGNAL(received(const QMQTT::Message&)), this, SLOT(received(const QMQTT::Message&)));
    
    QFont f = font();
    f.setBold(true);
    f.setPixelSize(25);
    tlabel->setFont(f);
    hlabel->setFont(f);
    m_noise->setFont(f);
    m_dewpoint->setFont(f);
    m_ssid->setFont(f);
    m_appid->setFont(f);
    m_rssi->setFont(f);
    m_uptime->setFont(f);
    m_rainToday->setFont(f);
    m_rainTotal->setFont(f);
    f.setPixelSize(25);
    m_lightning->setFont(f);
    f.setPixelSize(85);
    m_humidity->setFont(f);
    m_temp->setFont(f);

    QPalette pal(QColor(0,0,0));
    setBackgroundRole(QPalette::Window);
    pal.setColor(QPalette::Window, Qt::black);
    setAutoFillBackground(true);
    setPalette(pal);
    
    m_deviceMissing = true;
    m_eventTimer = new QTimer();
    connect(m_eventTimer, SIGNAL(timeout()), this, SLOT(missedEvents()));
    m_eventTimer->setInterval(1000 * 60 * 5);
}

PrimaryDisplay::~PrimaryDisplay()
{
}

void PrimaryDisplay::missedEvents()
{
    m_deviceMissing = true;
}

void PrimaryDisplay::layoutVisible (QGridLayout *layout, bool show)
{
    QLayoutItem *item = 0;  
    QWidget *widget = 0;
 
    for(int i = 0; i < layout->rowCount(); ++i) {
        for(int j = 0; j < layout->columnCount(); ++j) {
            item = layout->itemAtPosition(i,j);
            widget = item ? item->widget() : 0;
            if (widget)
                widget->setVisible(show);
        }
    }
}

void PrimaryDisplay::wakeUp()
{
    layoutVisible(m_layout, true);
    QDateTime now = QDateTime::currentDateTime();
    QTime t(0, 1, 0);
    QDateTime hideScreen(now.date().addDays(1), t);
    QTimer::singleShot(now.msecsTo(hideScreen), this, SLOT(goDark()));
}

void PrimaryDisplay::goDark()
{
    layoutVisible(m_layout, false);
    QDateTime now = QDateTime::currentDateTime();
    QTime t(5, 0, 0);
    QDateTime wakeup(now.date(), t);
    QTimer::singleShot(now.msecsTo(wakeup), this, SLOT(wakeUp()));
}

void PrimaryDisplay::showEvent(QShowEvent *e)
{
    Q_UNUSED(e)
    QDateTime now = QDateTime::currentDateTime();
    QTime t(0, 1, 0);
    QDateTime hideScreen(now.date().addDays(1), t);
    QTimer::singleShot(now.msecsTo(hideScreen), this, SLOT(goDark()));
}

//{"photon":{"id":"440018001151373331333230","version":"1.4.4","appid":62},"reset":{"reason":40},"time":{"timezone":-5,"now":1585585745},"network":{"ssid":"Office"},"device":{"AS3935":{}}}
void PrimaryDisplay::displayStartup(QByteArray payload)
{
    QJsonDocument doc = QJsonDocument::fromJson(payload);
    if (doc.isObject()) {
        QJsonObject parent = doc.object();
        QJsonObject network = parent["network"].toObject();
        QJsonObject photon = parent["photon"].toObject();
        
        m_ssid->setText(QString("SSID: ") + network["ssid"].toString());
        m_appid->setText(QString("Version: %1.%2").arg(photon["version"].toString()).arg(photon["appid"].toInt()));
    }
}

//{"device":{"AS3935":{"disturbers":"MASKED","indoor":"TRUE","noisefloor":2,"watchdog":2,"spikereject":8,"threshold":5}},"time":{"timezone":-5,"now":1585579482},"network":{"ssid":"Office"},"photon":{"id":"440018001151373331333230","version":"1.4.4","appid":60}}

void PrimaryDisplay::displayTunables(QByteArray payload)
{
    QJsonDocument doc = QJsonDocument::fromJson(payload);
    if (doc.isObject()) {
        QJsonObject parent = doc.object();
        QJsonObject device = parent["device"].toObject();
        QJsonObject network = parent["network"].toObject();
        QJsonObject AS3935 = device["AS3935"].toObject();
        QJsonObject photon = parent["photon"].toObject();
        
        m_noise->setText(QString("Noise Floor: %1").arg(AS3935["noisefloor"].toInt()));
//        m_threshold->setText(QString("Strike Threshold: %1").arg(AS3935["threshold"].toInt()));
        m_appid->setText(QString("Version: %1.%2").arg(photon["version"].toString()).arg(photon["appid"].toInt()));
    }
}

void PrimaryDisplay::hideLightning()
{
    m_lightning->hide();
}

// {"environment":{"celsius":25.37,"farenheit":77.71999,"humidity":34.3657},"time":1585591999}
void PrimaryDisplay::displayConditions(QByteArray payload)
{
    QJsonDocument doc = QJsonDocument::fromJson(payload);
    if (doc.isObject()) {
        QJsonObject parent = doc.object();
        QJsonObject values = parent["environment"].toObject();
        
        double t = values["farenheit"].toDouble();
        double h = values["humidity"].toDouble();
        double rt = values["raintoday"].toDouble();
        rt = rt * .023;
        double rain = values["raintotal"].toDouble();
        rain = rain *.023;
        
        if (m_deviceMissing) {
            m_temp->setStyleSheet("QLabel { color : red; }");
        }
        else {
            m_temp->setStyleSheet("QLabel { color : white; }");
        }
        QString temp = QString("%1%2").arg(t, 0, 'f', 1).arg(QChar(176));
        QString humidity = QString("%1%").arg(h, 0, 'f', 1);
        QString raintoday = QString("Rain Today: %1 in").arg(rt);
        QString raintotal = QString("Rain YTD: %1 in").arg(rain);
        m_dewpoint->setText(QString("Dewpoint: %1%2").arg(values["dewpointf"].toDouble(), 0, 'f', 1).arg(QChar(176)));
        m_temp->setText(temp);
        m_humidity->setText(humidity);
        m_rainToday->setText(raintoday);
        m_rainTotal->setText(raintotal);
    }
    else {
        qDebug() << __FUNCTION__ << ": Got bad json";
        qDebug() << payload;
    }
}

void PrimaryDisplay::displayLightningEvent(QByteArray payload)
{
    QJsonDocument doc = QJsonDocument::fromJson(payload);
    if (doc.isObject()) {
        QJsonObject event = doc.object();
        QJsonObject lightning = event["lightning"].toObject();
        
        int distance = lightning["distance"].toInt();
        QString label;
        if (distance == 1)
            label = QString("Lightning detected %1 mile away").arg(distance);
        else
            label = QString("Lightning detected %1 miles away").arg(distance);
        m_lightning->setText(label);
        m_lightning->show();
        QTimer::singleShot(1000 * 60, this, SLOT(hideLightning()));
    }
    else {
        qDebug() << __FUNCTION__ << ": Got bad json";
        qDebug() << payload;
    }
}

void PrimaryDisplay::dislplayNoisefloorEvent(QByteArray payload)
{
    QJsonDocument doc = QJsonDocument::fromJson(payload);
    if (doc.isObject()) {
        QJsonObject event = doc.object();
        QJsonObject floor = event["noisefloor"].toObject();

        m_noise->setText(QString("Noise Floor: %1").arg(floor["value"].toInt()));
        QTimer::singleShot(1000 * 60, this, SLOT(hideLightning()));
    }
    else {
        qDebug() << __FUNCTION__ << ": Got bad json";
        qDebug() << payload;
    }    
}

//{"network":{"ssid":"Office","signalquality":-61},"photon":{"freemem":42192,"uptime":313,"appid":77}}
void PrimaryDisplay::displayStats(QByteArray payload)
{
    QJsonDocument doc = QJsonDocument::fromJson(payload);
    if (doc.isObject()) {
        QJsonObject parent = doc.object();
        QJsonObject network = parent["network"].toObject();
        QJsonObject photon = parent["photon"].toObject();
        QJsonObject device = parent["device"].toObject();
        
        m_rssi->setText(QString("RSSI: %1").arg(network["signalquality"].toInt()));
        m_ssid->setText(QString("Memory: %1").arg(photon["freemem"].toInt()));

        QDateTime now = QDateTime::currentDateTime();
        QDateTime started = QDateTime::currentDateTime().addSecs(photon["uptime"].toInt() * -1);

        int seconds = started.secsTo(now);
        int hours = (seconds / 3600) % 24;
        int minutes = (seconds / 60) % 60;
        int days = (seconds / 86400);
        m_uptime->setText(QString("%1 d, %2 h, %3 m").arg(days).arg(hours).arg(minutes));
        m_appid->setText(QString("Version: %1.%2").arg(photon["version"].toString()).arg(photon["appid"].toInt()));
        m_noise->setText(QString("Noise Floor: %1").arg(device["noisefloor"].toInt()));
        m_ssid->setText(QString("SSID: ") + network["ssid"].toString());
    }
}

void PrimaryDisplay::connected()
{
    QMQTT::Message msg;
    qDebug() << __FUNCTION__;
    
    m_mqttClient->subscribe("weather/event/#");
    m_mqttClient->subscribe("weather/conditions");

    msg.setTopic("weather/request/tunables");
    m_mqttClient->publish(msg);
}

void PrimaryDisplay::disconnected()
{
}

void PrimaryDisplay::error(const QMQTT::ClientError error)
{
    qDebug() << __FUNCTION__ << ":" << error;
}

void PrimaryDisplay::pingresp()
{
}

void PrimaryDisplay::published(const quint16 msgid, const quint8 qos)
{
    Q_UNUSED(msgid)
    Q_UNUSED(qos)
}

void PrimaryDisplay::received(const QMQTT::Message& message)
{    
    m_deviceMissing = false;
    m_eventTimer->stop();
    m_eventTimer->start();
    
    if (message.topic() == "weather/conditions") {
        displayConditions(message.payload());
    }
    else if (message.topic() == "weather/event/lightning") {
        displayLightningEvent(message.payload());
    }
    else if (message.topic() == "weather/event/noisefloor") {
        dislplayNoisefloorEvent(message.payload());
    }
    else if (message.topic() == "weather/event/tunables") {
        displayTunables(message.payload());
    }
    else if (message.topic() == "weather/event/startup") {
        displayStartup(message.payload());
    }
    else if (message.topic() == "weather/event/system") {
        displayStats(message.payload());
    }
}

void PrimaryDisplay::subscribed(const QString& topic, const quint8 qos)
{
    Q_UNUSED(topic)
    Q_UNUSED(qos)
}

void PrimaryDisplay::unsubscribed(const QString& topic)
{
    Q_UNUSED(topic)
}
