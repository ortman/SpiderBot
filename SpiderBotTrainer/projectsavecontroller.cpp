#include "projectsavecontroller.h"
#include <QFile>
#include <QXmlStreamWriter>
#include <QDebug>

ProjectSaveController::ProjectSaveController(){
}

sbmFootStepInfo_t *parseStep(QXmlStreamReader *xml) {
    sbmFootStepInfo_t *stepInfo = static_cast<sbmFootStepInfo_t*>(malloc(sizeof(sbmFootStepInfo_t)));
    QVector<QVector<float>> angles;
    QXmlStreamReader::TokenType token = xml->readNext();
    while (!(token == QXmlStreamReader::EndElement && xml->name() == "step")) {
        if (token == QXmlStreamReader::StartElement) {
            if (xml->name() == "iterationCount") {
                token = xml->readNext();
                if (token == QXmlStreamReader::Characters) {
                    stepInfo->stepTimeIterations = xml->text().toUInt();
                }
            }
            if (xml->name() == "angles") {
                while (!(token == QXmlStreamReader::EndElement && xml->name() == "angles")) {
                    if (token == QXmlStreamReader::StartElement) {
                        if (xml->name() == "foot") {
                            token = xml->readNext();
                            if (token == QXmlStreamReader::Characters) {
                                QVector<float> segmentAngles;
                                QVector<QStringView> list = xml->text().split(';');
                                foreach(const QStringView &angle, list) {
                                    segmentAngles.append(angle.toFloat());
                                }
                                angles.append(segmentAngles);
                            }
                        }
                    }
                    token = xml->readNext();
                }
            }
        }
        token = xml->readNext();
    }
    sbmFootAngles_t* footAngles = static_cast<sbmFootAngles_t*>(malloc(sizeof(sbmFootAngles_t)));
    uint32_t footCount = angles.length();
    footAngles->footCount = footCount;
    footAngles->angles = static_cast<float**>(malloc(sizeof(float*) * footCount));
    footAngles->segmentsCount = static_cast<uint32_t*>(malloc(sizeof(uint32_t) * footCount));
    QVector<float> foot;
    for(uint32_t i=0; i<footCount; ++i) {
        foot = angles.at(i);
        uint32_t segmentCnt = foot.length();
        footAngles->segmentsCount[i] = segmentCnt;
        footAngles->angles[i] = static_cast<float*>(malloc(sizeof(float) * segmentCnt));
        for(uint32_t j=0; j<segmentCnt; ++j) {
            footAngles->angles[i][j] = foot.at(j);
        }
    }
    stepInfo->angles = footAngles;
    return stepInfo;
}

ProjectDataCommand *parseCommand(QXmlStreamReader *xml) {
    ProjectDataCommand *cmdData = new ProjectDataCommand();
    QString cmdName = xml->name().toString();
    cmdData->setCommandCode(static_cast<uint8_t>(cmdName.mid(5).toUInt(nullptr, 16)));
    QXmlStreamReader::TokenType token = xml->readNext();
    while (!(token == QXmlStreamReader::EndElement && xml->name() == cmdName)) {
        if (token == QXmlStreamReader::StartElement) {
            if (xml->name() == "interval") {
                token = xml->readNext();
                if (token == QXmlStreamReader::Characters) {
                    cmdData->setInterval(xml->text().toFloat());
                }
            }
            if (xml->name() == "name") {
                token = xml->readNext();
                if (token == QXmlStreamReader::Characters) {
                    cmdData->setCommandName(xml->text().toString());
                }
            }
            if (xml->name() == "step") {
                cmdData->addStep(parseStep(xml));
            }
        }
        token = xml->readNext();
    }
    return cmdData;
}

ProjectData *ProjectSaveController::load(QString fineName) {
    ProjectData *projData = new ProjectData();
    QFile *f = new QFile(fineName);
    if (!f->open(QIODevice::ReadOnly | QIODevice::Text)) {
        return projData;
    }
    QXmlStreamReader xml(f);
    QXmlStreamReader::TokenType token;
    while (!xml.atEnd() && !xml.hasError()) {
        token = xml.readNext();
        if (token == QXmlStreamReader::StartDocument) {
            continue;
        }
        if (token == QXmlStreamReader::StartElement) {
            if (xml.name() == "project") {
                QXmlStreamReader::TokenType token = xml.readNext();
                while (!(token == QXmlStreamReader::EndElement && xml.name() == "project")) {
                    if (token == QXmlStreamReader::StartElement) {
                        if (xml.name().startsWith(QString("CMD0x"))) {
                            ProjectDataCommand *cmd = parseCommand(&xml);
                            projData->addCommand(cmd);
                        }
                    }
                    if (token == QXmlStreamReader::StartElement) {
                        if (xml.name() == "lastActiveCammand") {
                            token = xml.readNext();
                            if (token == QXmlStreamReader::Characters) {
                                projData->setActiveCommand(xml.text().toInt());
                            }
                        }
                    }
                    token = xml.readNext();
                }
            }
        }
    }
    f->close();
    return projData;
}

void saveCommand(QXmlStreamWriter *xml, ProjectDataCommand *cmd) {
    xml->writeStartElement(QString("CMD0x").append(QString::number(cmd->getCommandCode(), 16)));
        xml->writeTextElement("interval", QString::number(cmd->getInterval()));
        xml->writeTextElement("name", cmd->getCommandName());
        sbmFootStepInfo_t *info;
        sbmFootAngles_t *angles;
        uint32_t footCount, segmentCount;
        uint32_t *segmentsCnts;
        uint32_t stepCount = cmd->getStepCount();
        for (uint32_t i=0; i<stepCount; ++i) {
            xml->writeStartElement("step");
                info = cmd->getStep(i);
                xml->writeTextElement("iterationCount", QString::number(info->stepTimeIterations));
                angles = info->angles;
                footCount = angles->footCount;
                segmentsCnts = angles->segmentsCount;
                xml->writeStartElement("angles");
                for (uint32_t footIndex=0; footIndex<footCount; ++footIndex) {
                    segmentCount = segmentsCnts[footIndex];
                    QString anglesData;
                    for (uint32_t segmentIndex=0; segmentIndex<segmentCount; ++segmentIndex) {
                        if (segmentIndex) {
                            anglesData.append(';');
                        }
                        anglesData.append(QString::number(angles->angles[footIndex][segmentIndex]));
                    }
                    xml->writeTextElement("foot", anglesData);
                }
                xml->writeEndElement();
            xml->writeEndElement();
        }
    xml->writeEndElement();
}

void ProjectSaveController::save(QString fineName, ProjectData *data) {
    QFile *f = new QFile(fineName);
    f->open(QIODevice::WriteOnly);
    QXmlStreamWriter xml(f);
    xml.setAutoFormatting(true);
    xml.writeStartDocument();
    xml.writeStartElement("project");
        int32_t commandCount = data->getCommandCount();
        for(int32_t i=0; i<commandCount; ++i) {
            saveCommand(&xml, data->getCommand(i));
        }
        xml.writeTextElement("lastActiveCammand", QString::number(data->getActiveCommandIndex()));
    xml.writeEndElement();
    f->close();
}

