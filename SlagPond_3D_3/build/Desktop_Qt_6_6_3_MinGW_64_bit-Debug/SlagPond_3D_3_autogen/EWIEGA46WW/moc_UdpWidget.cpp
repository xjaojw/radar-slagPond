/****************************************************************************
** Meta object code from reading C++ file 'UdpWidget.h'
**
** Created by: The Qt Meta Object Compiler version 68 (Qt 6.6.3)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../../../UdpWidget.h"
#include <QtCore/qmetatype.h>

#if __has_include(<QtCore/qtmochelpers.h>)
#include <QtCore/qtmochelpers.h>
#else
QT_BEGIN_MOC_NAMESPACE
#endif


#include <memory>

#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'UdpWidget.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 68
#error "This file was generated using the moc from 6.6.3. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

#ifndef Q_CONSTINIT
#define Q_CONSTINIT
#endif

QT_WARNING_PUSH
QT_WARNING_DISABLE_DEPRECATED
QT_WARNING_DISABLE_GCC("-Wuseless-cast")
namespace {

#ifdef QT_MOC_HAS_STRINGDATA
struct qt_meta_stringdata_CLASSUdpWidgetENDCLASS_t {};
constexpr auto qt_meta_stringdata_CLASSUdpWidgetENDCLASS = QtMocHelpers::stringData(
    "UdpWidget",
    "dataReceived",
    "",
    "data",
    "QHostAddress",
    "sender",
    "senderPort",
    "socketErrorOccurred",
    "errorString",
    "bindingStatusChanged",
    "isBound",
    "port",
    "sendMessage",
    "clearLog",
    "onSendButtonClicked",
    "onSocketReadyRead",
    "onBindButtonClicked",
    "onClearLogButtonClicked"
);
#else  // !QT_MOC_HAS_STRING_DATA
struct qt_meta_stringdata_CLASSUdpWidgetENDCLASS_t {
    uint offsetsAndSizes[36];
    char stringdata0[10];
    char stringdata1[13];
    char stringdata2[1];
    char stringdata3[5];
    char stringdata4[13];
    char stringdata5[7];
    char stringdata6[11];
    char stringdata7[20];
    char stringdata8[12];
    char stringdata9[21];
    char stringdata10[8];
    char stringdata11[5];
    char stringdata12[12];
    char stringdata13[9];
    char stringdata14[20];
    char stringdata15[18];
    char stringdata16[20];
    char stringdata17[24];
};
#define QT_MOC_LITERAL(ofs, len) \
    uint(sizeof(qt_meta_stringdata_CLASSUdpWidgetENDCLASS_t::offsetsAndSizes) + ofs), len 
Q_CONSTINIT static const qt_meta_stringdata_CLASSUdpWidgetENDCLASS_t qt_meta_stringdata_CLASSUdpWidgetENDCLASS = {
    {
        QT_MOC_LITERAL(0, 9),  // "UdpWidget"
        QT_MOC_LITERAL(10, 12),  // "dataReceived"
        QT_MOC_LITERAL(23, 0),  // ""
        QT_MOC_LITERAL(24, 4),  // "data"
        QT_MOC_LITERAL(29, 12),  // "QHostAddress"
        QT_MOC_LITERAL(42, 6),  // "sender"
        QT_MOC_LITERAL(49, 10),  // "senderPort"
        QT_MOC_LITERAL(60, 19),  // "socketErrorOccurred"
        QT_MOC_LITERAL(80, 11),  // "errorString"
        QT_MOC_LITERAL(92, 20),  // "bindingStatusChanged"
        QT_MOC_LITERAL(113, 7),  // "isBound"
        QT_MOC_LITERAL(121, 4),  // "port"
        QT_MOC_LITERAL(126, 11),  // "sendMessage"
        QT_MOC_LITERAL(138, 8),  // "clearLog"
        QT_MOC_LITERAL(147, 19),  // "onSendButtonClicked"
        QT_MOC_LITERAL(167, 17),  // "onSocketReadyRead"
        QT_MOC_LITERAL(185, 19),  // "onBindButtonClicked"
        QT_MOC_LITERAL(205, 23)   // "onClearLogButtonClicked"
    },
    "UdpWidget",
    "dataReceived",
    "",
    "data",
    "QHostAddress",
    "sender",
    "senderPort",
    "socketErrorOccurred",
    "errorString",
    "bindingStatusChanged",
    "isBound",
    "port",
    "sendMessage",
    "clearLog",
    "onSendButtonClicked",
    "onSocketReadyRead",
    "onBindButtonClicked",
    "onClearLogButtonClicked"
};
#undef QT_MOC_LITERAL
#endif // !QT_MOC_HAS_STRING_DATA
} // unnamed namespace

Q_CONSTINIT static const uint qt_meta_data_CLASSUdpWidgetENDCLASS[] = {

 // content:
      12,       // revision
       0,       // classname
       0,    0, // classinfo
       9,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       3,       // signalCount

 // signals: name, argc, parameters, tag, flags, initial metatype offsets
       1,    3,   68,    2, 0x06,    1 /* Public */,
       7,    1,   75,    2, 0x06,    5 /* Public */,
       9,    2,   78,    2, 0x06,    7 /* Public */,

 // slots: name, argc, parameters, tag, flags, initial metatype offsets
      12,    0,   83,    2, 0x0a,   10 /* Public */,
      13,    0,   84,    2, 0x0a,   11 /* Public */,
      14,    0,   85,    2, 0x08,   12 /* Private */,
      15,    0,   86,    2, 0x08,   13 /* Private */,
      16,    0,   87,    2, 0x08,   14 /* Private */,
      17,    0,   88,    2, 0x08,   15 /* Private */,

 // signals: parameters
    QMetaType::Void, QMetaType::QByteArray, 0x80000000 | 4, QMetaType::UShort,    3,    5,    6,
    QMetaType::Void, QMetaType::QString,    8,
    QMetaType::Void, QMetaType::Bool, QMetaType::UShort,   10,   11,

 // slots: parameters
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,

       0        // eod
};

Q_CONSTINIT const QMetaObject UdpWidget::staticMetaObject = { {
    QMetaObject::SuperData::link<QWidget::staticMetaObject>(),
    qt_meta_stringdata_CLASSUdpWidgetENDCLASS.offsetsAndSizes,
    qt_meta_data_CLASSUdpWidgetENDCLASS,
    qt_static_metacall,
    nullptr,
    qt_incomplete_metaTypeArray<qt_meta_stringdata_CLASSUdpWidgetENDCLASS_t,
        // Q_OBJECT / Q_GADGET
        QtPrivate::TypeAndForceComplete<UdpWidget, std::true_type>,
        // method 'dataReceived'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<const QByteArray &, std::false_type>,
        QtPrivate::TypeAndForceComplete<const QHostAddress &, std::false_type>,
        QtPrivate::TypeAndForceComplete<quint16, std::false_type>,
        // method 'socketErrorOccurred'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<const QString &, std::false_type>,
        // method 'bindingStatusChanged'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<bool, std::false_type>,
        QtPrivate::TypeAndForceComplete<quint16, std::false_type>,
        // method 'sendMessage'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'clearLog'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'onSendButtonClicked'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'onSocketReadyRead'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'onBindButtonClicked'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'onClearLogButtonClicked'
        QtPrivate::TypeAndForceComplete<void, std::false_type>
    >,
    nullptr
} };

void UdpWidget::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        auto *_t = static_cast<UdpWidget *>(_o);
        (void)_t;
        switch (_id) {
        case 0: _t->dataReceived((*reinterpret_cast< std::add_pointer_t<QByteArray>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<QHostAddress>>(_a[2])),(*reinterpret_cast< std::add_pointer_t<quint16>>(_a[3]))); break;
        case 1: _t->socketErrorOccurred((*reinterpret_cast< std::add_pointer_t<QString>>(_a[1]))); break;
        case 2: _t->bindingStatusChanged((*reinterpret_cast< std::add_pointer_t<bool>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<quint16>>(_a[2]))); break;
        case 3: _t->sendMessage(); break;
        case 4: _t->clearLog(); break;
        case 5: _t->onSendButtonClicked(); break;
        case 6: _t->onSocketReadyRead(); break;
        case 7: _t->onBindButtonClicked(); break;
        case 8: _t->onClearLogButtonClicked(); break;
        default: ;
        }
    } else if (_c == QMetaObject::IndexOfMethod) {
        int *result = reinterpret_cast<int *>(_a[0]);
        {
            using _t = void (UdpWidget::*)(const QByteArray & , const QHostAddress & , quint16 );
            if (_t _q_method = &UdpWidget::dataReceived; *reinterpret_cast<_t *>(_a[1]) == _q_method) {
                *result = 0;
                return;
            }
        }
        {
            using _t = void (UdpWidget::*)(const QString & );
            if (_t _q_method = &UdpWidget::socketErrorOccurred; *reinterpret_cast<_t *>(_a[1]) == _q_method) {
                *result = 1;
                return;
            }
        }
        {
            using _t = void (UdpWidget::*)(bool , quint16 );
            if (_t _q_method = &UdpWidget::bindingStatusChanged; *reinterpret_cast<_t *>(_a[1]) == _q_method) {
                *result = 2;
                return;
            }
        }
    }
}

const QMetaObject *UdpWidget::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *UdpWidget::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_CLASSUdpWidgetENDCLASS.stringdata0))
        return static_cast<void*>(this);
    return QWidget::qt_metacast(_clname);
}

int UdpWidget::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QWidget::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 9)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 9;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 9)
            *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType();
        _id -= 9;
    }
    return _id;
}

// SIGNAL 0
void UdpWidget::dataReceived(const QByteArray & _t1, const QHostAddress & _t2, quint16 _t3)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))), const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t2))), const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t3))) };
    QMetaObject::activate(this, &staticMetaObject, 0, _a);
}

// SIGNAL 1
void UdpWidget::socketErrorOccurred(const QString & _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))) };
    QMetaObject::activate(this, &staticMetaObject, 1, _a);
}

// SIGNAL 2
void UdpWidget::bindingStatusChanged(bool _t1, quint16 _t2)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))), const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t2))) };
    QMetaObject::activate(this, &staticMetaObject, 2, _a);
}
QT_WARNING_POP
