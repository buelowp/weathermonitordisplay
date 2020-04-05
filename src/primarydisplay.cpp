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
    m_threshold = new QLabel();
    m_ssid = new QLabel();
    m_appid = new QLabel();
    m_rssi = new QLabel();
    m_uptime = new QLabel();
    
    QLabel *tlabel = new QLabel("Temperature");
    QLabel *hlabel = new QLabel("Humidity");
    
    m_layout = new QGridLayout();
    m_layout->addWidget(tlabel, 0, 0, Qt::AlignCenter);
    m_layout->addWidget(hlabel, 0, 1, Qt::AlignCenter);
    m_layout->addWidget(m_temp, 1, 0, 2, 1, Qt::AlignCenter);
    m_layout->addWidget(m_humidity, 1, 1, 2, 1, Qt::AlignCenter);
    m_layout->addWidget(m_lightning, 3, 0, 1, 2, Qt::AlignCenter);
    m_layout->addWidget(m_noise, 4, 0, Qt::AlignCenter);
    m_layout->addWidget(m_threshold, 4, 1, Qt::AlignCenter);
    m_layout->addWidget(m_ssid, 5, 0, Qt::AlignCenter);
    m_layout->addWidget(m_appid, 5, 1, Qt::AlignCenter);
    m_layout->addWidget(m_rssi, 6, 0, Qt::AlignCenter);
    m_layout->addWidget(m_uptime, 6, 1, Qt::AlignCenter);
    m_layout->setContentsMargins(0, 0, 0, 0);
    m_layout->setSpacing(0);
    setLayout(m_layout);
    
    m_mqttClient = new QMQTT::Client("mqttserver.home", 1883, false, false);
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
    f.setPixelSize(20);
    tlabel->setFont(f);
    hlabel->setFont(f);
    m_noise->setFont(f);
    m_threshold->setFont(f);
    m_ssid->setFont(f);
    m_appid->setFont(f);
    m_rssi->setFont(f);
    m_uptime->setFont(f);
    f.setPixelSize(90);
    m_humidity->setFont(f);
    m_temp->setFont(f);
    
    QPalette pal(QColor(0,0,0));
	setBackgroundRole(QPalette::Window);
	pal.setColor(QPalette::Window, Qt::black);
	setAutoFillBackground(true);
	setPalette(pal);
}

PrimaryDisplay::~PrimaryDisplay()
{
}

void PrimaryDisplay::showEvent(QShowEvent *e)
{
    Q_UNUSED(e)
}

//{"photon":{"id":"440018001151373331333230","version":"1.4.4","appid":62},"reset":{"reason":40},"time":{"timezone":-5,"now":1585585745},"network":{"ssid":"Office"},"device":{"AS3935":{}}}
void PrimaryDisplay::displayStartup(QByteArray payload)
{
    QMQTT::Message msg;
    QJsonDocument doc = QJsonDocument::fromJson(payload);
    if (doc.isObject()) {
        QJsonObject parent = doc.object();
        QJsonObject network = parent["network"].toObject();
        QJsonObject photon = parent["photon"].toObject();
        
        m_ssid->setText(QString("SSID: ") + network["ssid"].toString());
        m_appid->setText(QString("Version: %1.%2").arg(photon["version"].toString()).arg(photon["appid"].toInt()));
    }
    
    msg.setTopic("weather/request/tunables");
    m_mqttClient->publish(msg);
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
        m_threshold->setText(QString("Strike Threshold: %1").arg(AS3935["threshold"].toInt()));
        m_ssid->setText(QString("SSID: ") + network["ssid"].toString());
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
        
        QString temp = QString("%1%2").arg(t, 0, 'f', 1).arg(QChar(176));
        QString humidity = QString("%1%").arg(h, 0, 'f', 1);
        m_temp->setText(temp);
        m_humidity->setText(humidity);
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
        QString label = QString("Lightning detected %1 miles away").arg(distance);
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
        
        m_rssi->setText(QString("RSSI: %1").arg(network["signalquality"].toInt()));
        QDateTime now = QDateTime::currentDateTime();
        QDateTime started = QDateTime::currentDateTime().addSecs(photon["uptime"].toInt() * -1);
        qDebug() << __FUNCTION__ << ":" << photon["uptime"].toInt();
        int days = started.daysTo(now);
        int seconds = started.secsTo(now);
        int hours = (seconds / 3600) % 24;
        int minutes = (seconds / 60) % 60;
        m_uptime->setText(QString("%1 d, %2 h, %3 m").arg(days).arg(hours).arg(minutes));
    }
}

void PrimaryDisplay::connected()
{
    QMQTT::Message msg;
    qDebug() << __FUNCTION__;
    msg.setTopic("weather/request/tunables");
    
    m_mqttClient->subscribe("weather/event/#");
    m_mqttClient->subscribe("weather/conditions");
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
    else {
        qDebug() << __FUNCTION__ << ": Got a message on topic" << message.topic();
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
