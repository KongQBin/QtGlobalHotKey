/****************************************************************************
 **
 ** Copyright (C) Qxt Foundation. Some rights reserved.
 **
 ** This file is part of the QxtGui module of the Qxt library.
 **
 ** This library is free software; you can redistribute it and/or modify it
 ** under the terms of the Common Public License, version 1.0, as published
 ** by IBM, and/or under the terms of the GNU Lesser General Public License,
 ** version 2.1, as published by the Free Software Foundation.
 **
 ** This file is provided "AS IS", without WARRANTIES OR CONDITIONS OF ANY
 ** KIND, EITHER EXPRESS OR IMPLIED INCLUDING, WITHOUT LIMITATION, ANY
 ** WARRANTIES OR CONDITIONS OF TITLE, NON-INFRINGEMENT, MERCHANTABILITY OR
 ** FITNESS FOR A PARTICULAR PURPOSE.
 **
 ** You should have received a copy of the CPL and the LGPL along with this
 ** file. See the LICENSE file and the cpl1.0.txt/lgpl-2.1.txt files
 ** included with the source distribution for more information.
 ** If you did not receive a copy of the licenses, contact the Qxt Foundation.
 **
 ** <http://libqxt.org>  <foundation@libqxt.org>
 **
 ****************************************************************************/
#include "qxtglobalshortcut_p.h"
#include <QX11Info>
#include <xcb/xcb.h>
#include <X11/Xlib.h>
#include <cstdio>

static int (*original_x_errhandler)(Display* display, XErrorEvent* event);

static int qxt_x_errhandler(Display* display, XErrorEvent *event)
{
    Q_UNUSED(display);
    switch (event->error_code)
    {
    case BadAccess:
    case BadValue:
    case BadWindow:
        if (event->request_code == 33 /* X_GrabKey */ ||
                event->request_code == 34 /* X_UngrabKey */)
        {
            QxtGlobalShortcutPrivate::error = true;
            //TODO:
            //char errstr[256];
            //XGetErrorText(dpy, err->error_code, errstr, 256);
        }
    default:
        return 0;
    }
}
#if(QT_VERSION<0x050000)
bool QxtGlobalShortcutPrivate::eventFilter(void* message)
{
    XEvent* event = static_cast<XEvent*>(message);
    if (event->type == KeyPress)
    {
        XKeyEvent* key = (XKeyEvent*) event;
        activateShortcut(key->keycode,
                         // Mod1Mask == Alt, Mod4Mask == Meta
                         key->state & (ShiftMask | ControlMask | Mod1Mask | Mod4Mask));
    }
    return false;
}
#else
bool QxtGlobalShortcutPrivate::nativeEventFilter(const QByteArray &qbyte, void *message, long *l)
{
    if(qbyte == "xcb_generic_event_t")
    {

        xcb_generic_event_t* e = static_cast<xcb_generic_event_t*>(message);
        if(!e->pad0) return false;
//        qDebug()<<"**************************************************************************  ";
//        for(int i = 0;i<7;++i)
//        {
//            qDebug()<<e->pad[i];
//        }
        /*
 *      qDebug()<<e->response_type;
 *      qDebug()<<e->pad0;
 *      qDebug()<<e->sequence;
 *      qDebug()<<e->full_sequence;
 *      *******************************按下时************************************
 *      2
 *      74
 *      695
 *      695
 *      *******************************弹出时************************************
 *      3
 *      74
 *      699
 *      699
*/
//        qDebug()<<e->pad[5];
        if(!(e->response_type%2))
            activateShortcut(e->pad0,e->pad[6]&(ControlMask|Mod1Mask));


        //        XEvent* event = static_cast<XEvent*>(message);
        //        XKeyEvent* key = (XKeyEvent*) event;
        //        qDebug()<<key->keycode<<key->same_screen<<key->send_event<<key->serial<<key->state
        //               <<key->time<<key->type;


        /*
**************************************************************************
nativeKey =  33
nativeMods =  4
0 1361 14681790 5059471475866 567 1178 47653122
**************************************************************************
nativeKey =  0
nativeMods =  56
0 1361 0 103829069149241681 567 1178 47775772
**************************************************************************
nativeKey =  33
nativeMods =  6
0 1361 14681790 5059471475866 567 1178 47915267
**************************************************************************
nativeKey =  33
nativeMods =  4
0 1361 14681790 5059471475866 567 1178 47915266
**************************************************************************
nativeKey =  33
nativeMods =  6
0 1361 14681790 5059471475866 567 1178 47980803
**************************************************************************
nativeKey =  33
nativeMods =  4
0 1361 14681790 5059471475866 567 1178 47980802
**************************************************************************
nativeKey =  33
nativeMods =  6
0 1361 14681790 5059471475866 567 1178 48046339
**************************************************************************
nativeKey =  33
nativeMods =  4
0 1361 14681790 5059471475866 567 1178 48046338
**************************************************************************
nativeKey =  33
nativeMods =  6
0 1361 14681790 5059471475866 567 1178 48111875
**************************************************************************
nativeKey =  33
nativeMods =  4
0 1361 14681790 5059471475866 567 1178 48111874
**************************************************************************
nativeKey =  33
nativeMods =  6
0 1361 14681790 5059471475866 567 1178 48177411
*/

        //                if(CTRL_ADD_F8 == event->xbutton.serial && !(event->type%2)/*键盘按下时，数值为偶。键盘弹出时，数值为奇。*/)
        //                {
        //                    XKeyEvent* key = (XKeyEvent*) event;
        //                            activateShortcut(key->keycode,key->state & (ShiftMask | ControlMask | Mod1Mask | Mod4Mask));
        //                }
        //                else
        //                {
        //                    qDebug()<<event->xbutton.serial;
        //                }



        /////////////////////原代码////////////////////
        //                XEvent* event = static_cast<XEvent*>(message);
        //                if (event->type == KeyPress)
        //                {
        //                    qDebug()<<"event->type == KeyPress";
        //                    XKeyEvent* key = (XKeyEvent*) event;
        //                    activateShortcut(key->keycode,
        //                                     // Mod1Mask == Alt, Mod4Mask == Meta
        //                                     key->state & (ShiftMask | ControlMask | Mod1Mask | Mod4Mask));
        //                }


    }
    return false;
}
#endif

quint32 QxtGlobalShortcutPrivate::nativeModifiers(Qt::KeyboardModifiers modifiers)
{
    // ShiftMask, LockMask, ControlMask, Mod1Mask, Mod2Mask, Mod3Mask, Mod4Mask, and Mod5Mask
    quint32 native = 0;
    if (modifiers & Qt::ShiftModifier)
        native |= ShiftMask;
    if (modifiers & Qt::ControlModifier)
        native |= ControlMask;
    if (modifiers & Qt::AltModifier)
        native |= Mod1Mask;
    // TODO: resolve these?
    //if (modifiers & Qt::MetaModifier)
    //if (modifiers & Qt::KeypadModifier)
    //if (modifiers & Qt::GroupSwitchModifier)
    return native;
}

quint32 QxtGlobalShortcutPrivate::nativeKeycode(Qt::Key key)
{
    Display* display = QX11Info::display();
    return XKeysymToKeycode(display, XStringToKeysym(QKeySequence(key).toString().toLatin1().data()));
}

bool QxtGlobalShortcutPrivate::registerShortcut(quint32 nativeKey, quint32 nativeMods)
{
    Display* display = QX11Info::display();
    Window window = QX11Info::appRootWindow();
    Bool owner = True;
    int pointer = GrabModeAsync;
    int keyboard = GrabModeAsync;
    error = false;
    original_x_errhandler = XSetErrorHandler(qxt_x_errhandler);
    XGrabKey(display, nativeKey, nativeMods, window, owner, pointer, keyboard);
    XGrabKey(display, nativeKey, nativeMods | Mod2Mask, window, owner, pointer, keyboard); // allow numlock
    XSync(display, False);
    XSetErrorHandler(original_x_errhandler);
    return !error;
}

bool QxtGlobalShortcutPrivate::unregisterShortcut(quint32 nativeKey, quint32 nativeMods)
{
    Display* display = QX11Info::display();
    Window window = QX11Info::appRootWindow();
    error = false;
    original_x_errhandler = XSetErrorHandler(qxt_x_errhandler);
    XUngrabKey(display, nativeKey, nativeMods, window);
    XUngrabKey(display, nativeKey, nativeMods | Mod2Mask, window); // allow numlock
    XSync(display, False);
    XSetErrorHandler(original_x_errhandler);
    return !error;
}
