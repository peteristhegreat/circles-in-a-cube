/****************************************************************************
**
** Copyright (C) 2012 Nokia Corporation and/or its subsidiary(-ies).
** All rights reserved.
** Contact: Nokia Corporation (qt-info@nokia.com)
**
** This file is part of the examples of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:BSD$
** You may use this file under the terms of the BSD license as follows:
**
** "Redistribution and use in source and binary forms, with or without
** modification, are permitted provided that the following conditions are
** met:
**   * Redistributions of source code must retain the above copyright
**     notice, this list of conditions and the following disclaimer.
**   * Redistributions in binary form must reproduce the above copyright
**     notice, this list of conditions and the following disclaimer in
**     the documentation and/or other materials provided with the
**     distribution.
**   * Neither the name of Nokia Corporation and its Subsidiary(-ies) nor
**     the names of its contributors may be used to endorse or promote
**     products derived from this software without specific prior written
**     permission.
**
** THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
** "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
** LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
** A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
** OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
** SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
** LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
** DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
** THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
** (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
** OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE."
** $QT_END_LICENSE$
**
****************************************************************************/

#include <QtGui>
#include <QtOpenGL>

#include <QtGlobal>
#include <math.h>

#ifdef Q_OS_WIN
#include "gl/GLU.h"
#else
#include <glu.h>
#endif
#include "glwidget.h"
#include <QDebug>
#include <QDateTime>

#define PI 3.1459

GLWidget::GLWidget(QWidget *parent)
    : QGLWidget(parent)
{
    qDebug() << Q_FUNC_INFO;

    if(true)
    {
        xRot = 468;
        yRot = 5064;
        zRot = 0;
        m_panX = -10;
        m_panY = 6.375;
        m_time = 0;
        timeBasedVar = 0;
        zoomFactor = -70;
    }
    else
    {
        xRot = 0;
        yRot = 0;
        zRot = 0;
        m_panX = 0;
        m_panY = 0;
        m_time = 0;
        timeBasedVar = 0;
        zoomFactor = -30;
    }
    int countPerSide = 8;


    for(int x = 0; x < countPerSide; x++)
    {
        spheresInSpace.append(QVector<QVector <MySphere *> >());
        for(int y = 0; y < countPerSide; y++)
        {
            spheresInSpace[x].append(QVector<MySphere*>());
            for(int z = 0; z < countPerSide; z++)
            {
                MySphere * sph = new MySphere();
                sph->center = QVertex(x,y,z);
                sph->radius = 0.3;
                allSpheres.append(sph);
                spheresInSpace[x][y].append(sph);
            }
        }
    }

    int mid = countPerSide/2;
    center = spheresInSpace[mid][mid][mid]->center;


//    grabKeyboard();

//    QTimer *timer = new QTimer(this);
//    connect(timer, SIGNAL(timeout()), this, SLOT(advanceGears()));
//    timer->start(20);
}

GLWidget::~GLWidget()
{
    makeCurrent();

    foreach( MySphere * s, allSpheres)
    {
        glDeleteLists(s->glQuadric, 1);
    }

    qDeleteAll(allSpheres);
    allSpheres.clear();
}

void GLWidget::setXRotation(int angle)
{
    normalizeAngle(&angle);
    if (angle != xRot) {
        xRot = angle;
        emit xRotationChanged(angle);
        updateGL();
    }
}

void GLWidget::setYRotation(int angle)
{
    normalizeAngle(&angle);
    if (angle != yRot) {
        yRot = angle;
        emit yRotationChanged(angle);
        updateGL();
    }
}

void GLWidget::setZRotation(int angle)
{
    normalizeAngle(&angle);
    if (angle != zRot) {
        zRot = angle;
        emit zRotationChanged(angle);
        updateGL();
    }
}

void GLWidget::setReflectance(GLfloat * glcolor, QColor color)
{
    glcolor[0] = color.redF();
    glcolor[2] = color.blueF();
    glcolor[1] = color.greenF();
    glcolor[3] = color.alphaF();
}

void GLWidget::initializeGL()
{
    qDebug() << Q_FUNC_INFO;
    static const GLfloat lightPos[4] = { 5.0f, 5.0f, 10.0f, 1.0f };

    glLightfv(GL_LIGHT0, GL_POSITION, lightPos);
    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);
    glEnable(GL_DEPTH_TEST);

    if(false)
    {
        // Assign a random color to each sphere
        QList <QColor> colors;
        colors << Qt::red
               << QColor("orange")
               << Qt::yellow
               << Qt::green
               << Qt::blue
               << QColor("violet");
        qsrand((int)QDateTime::currentMSecsSinceEpoch());

        foreach(MySphere * s, allSpheres)
        {
            setReflectance(s->reflectance, colors.at(qrand()%colors.size()));
            s->glQuadric = makeSphere(*s);
        }
    }
    else
    {
        int side = spheresInSpace.size();
        for(int x = 0; x < spheresInSpace.size(); x++)
        {
            for(int y = 0; y < spheresInSpace.at(x).size(); y++)
            {
                for(int z = 0; z < spheresInSpace.at(x).at(y).size(); z++)
                {
                    MySphere * s = spheresInSpace[x][y][z];
                    setReflectance(s->reflectance, QColor((qreal)x/side*255, (qreal)y/side*255, (qreal)z/side*255));
                    s->glQuadric = makeSphere(*s);
                }
            }
        }
    }

    glEnable(GL_NORMALIZE);
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
}

void GLWidget::paintGL()
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glLoadIdentity();

    glPushMatrix();
    glTranslated(m_panX, m_panY, zoomFactor);
    glRotated(xRot / 16.0, 1.0, 0.0, 0.0);
    glRotated(yRot / 16.0, 0.0, 1.0, 0.0);
    glRotated(zRot / 16.0, 0.0, 0.0, 1.0);

    glTranslated(center.x(), center.y(), center.z());
//    qDebug() << m_panX << m_panY << zoomFactor << xRot << yRot << zRot;
    m_time += 0.05;
    glRotated(+180.0, 1.0, 0.0, 0.0);

    foreach(MySphere * s, allSpheres)
    {
        drawSphere(*s);
    }

    glPopMatrix();
}

void GLWidget::resizeGL(int width, int height)
{
//    qDebug() << Q_FUNC_INFO;
    if(false)
    {
        // make the viewport a fitted square
        int side = qMin(width, height);
        glViewport((width - side) / 2, (height - side) / 2, side, side);
    }
    else
    {
        glViewport(0,0,width,height);
    }

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    // left, right, top bottom near far
    glFrustum(-1.0*(qreal)width/(qreal)height, +1.0*(qreal)width/(qreal)height, -1.0, 1.0, 5.0, 200.0);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glTranslated(0.0, 0.0, -40.0);
}

void GLWidget::wheelEvent(QWheelEvent *e)
{
    zoomFactor += e->delta()/15/8;

    updateGL();
}

void GLWidget::mousePressEvent(QMouseEvent *event)
{
    lastPos = event->pos();
}

void GLWidget::mouseMoveEvent(QMouseEvent *event)
{
    qreal dx = event->x() - lastPos.x();
    qreal dy = event->y() - lastPos.y();

    if (event->buttons() & Qt::MiddleButton) {
        setXRotation(xRot + 8 * dy);
        setYRotation(yRot + 8 * dx);
    } else if (event->buttons() & Qt::RightButton) {
        setXRotation(xRot + 8 * dy);
        setZRotation(zRot + 8 * dx);
    } else if (event->buttons() & Qt::LeftButton) {
        m_panX += dx/16;
        m_panY -= dy/16;
    }
    lastPos = event->pos();
    updateGL();
}

void GLWidget::advanceGears()
{
    timeBasedVar += 2 * 16;
    updateGL();
}

GLuint GLWidget::makeSphere(MySphere s)
{
    GLuint list = glGenLists(1);
    glNewList(list, GL_COMPILE);
    glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, s.reflectance);

    glShadeModel(GL_FLAT);
    GLUquadric * sphere = gluNewQuadric();

    int slices = 15;
    int stacks = slices*2/3;

    glTranslated(s.center.x(), s.center.y(), s.center.z());
    gluSphere(sphere, s.radius, slices, stacks);
    glEndList();

    return list;
}

void GLWidget::drawSphere(MySphere s)
{
    glPushMatrix();
    glTranslated(s.center.x(), s.center.y(), s.center.z());
    glCallList(s.glQuadric);
    glPopMatrix();
}

void GLWidget::drawPendulum(Pendulum p,
                        GLdouble angle)
{
    glPushMatrix();
    glTranslated(p.pt.x(), p.pt.y(), p.pt.z());
    glRotated(angle, 0.0, 0.0, 1.0);
    glCallList(p.glQuadric);
    glPopMatrix();
}

void GLWidget::normalizeAngle(int *angle)
{
    while (*angle < 0)
        *angle += 360 * 16;
    while (*angle > 360 * 16)
        *angle -= 360 * 16;
}
