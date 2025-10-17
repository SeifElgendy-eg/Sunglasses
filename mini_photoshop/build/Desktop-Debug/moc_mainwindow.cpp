/****************************************************************************
** Meta object code from reading C++ file 'mainwindow.h'
**
** Created by: The Qt Meta Object Compiler version 69 (Qt 6.9.2)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../mainwindow.h"
#include <QtGui/qtextcursor.h>
#include <QtCore/qmetatype.h>

#include <QtCore/qtmochelpers.h>

#include <memory>


#include <QtCore/qxptype_traits.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'mainwindow.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 69
#error "This file was generated using the moc from 6.9.2. It"
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
struct qt_meta_tag_ZN10MainWindowE_t {};
} // unnamed namespace

template <> constexpr inline auto MainWindow::qt_create_metaobjectdata<qt_meta_tag_ZN10MainWindowE_t>()
{
    namespace QMC = QtMocConstants;
    QtMocHelpers::StringRefStorage qt_stringData {
        "MainWindow",
        "applyMorphFilter",
        "",
        "blendFactor",
        "applyMorphAnimated",
        "frameCount",
        "applyMergeToCanvas",
        "openImage",
        "saveImage",
        "showApplyCancelButtons",
        "hideApplyCancelButtons",
        "exitApp",
        "onLoadImage",
        "onSaveImage",
        "onResetImage",
        "onUndo",
        "onRedo",
        "onSelectTool",
        "onResizeTool",
        "onCropTool",
        "onCanvasVerticalReflection",
        "onCanvasHorizontalReflection",
        "onCanvasYellowFilter",
        "onCanvasPurpleFilter",
        "onCanvasInfraRedFilter",
        "onCanvasUndo",
        "onCanvasRedo",
        "onCanvasReset",
        "onApplyBnWFilter",
        "onPreviewBnWFilter",
        "threshold",
        "onApplyBlurFilter",
        "onPreviewBlurFilter",
        "kernelSize",
        "onApplyLightOrDarkFilter",
        "onPreviewLightOrDarkFilter",
        "percent",
        "onApplyOilPaintFilter",
        "onPreviewOilPaintFilter",
        "onApplySkewFilter",
        "onPreviewSkewFilter",
        "degree"
    };

    QtMocHelpers::UintData qt_methods {
        // Slot 'applyMorphFilter'
        QtMocHelpers::SlotData<void(double)>(1, 2, QMC::AccessPrivate, QMetaType::Void, {{
            { QMetaType::Double, 3 },
        }}),
        // Slot 'applyMorphAnimated'
        QtMocHelpers::SlotData<void(int, double)>(4, 2, QMC::AccessPrivate, QMetaType::Void, {{
            { QMetaType::Int, 5 }, { QMetaType::Double, 3 },
        }}),
        // Slot 'applyMergeToCanvas'
        QtMocHelpers::SlotData<void()>(6, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'openImage'
        QtMocHelpers::SlotData<void()>(7, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'saveImage'
        QtMocHelpers::SlotData<void()>(8, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'showApplyCancelButtons'
        QtMocHelpers::SlotData<void()>(9, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'hideApplyCancelButtons'
        QtMocHelpers::SlotData<void()>(10, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'exitApp'
        QtMocHelpers::SlotData<void()>(11, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'onLoadImage'
        QtMocHelpers::SlotData<void()>(12, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'onSaveImage'
        QtMocHelpers::SlotData<void()>(13, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'onResetImage'
        QtMocHelpers::SlotData<void()>(14, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'onUndo'
        QtMocHelpers::SlotData<void()>(15, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'onRedo'
        QtMocHelpers::SlotData<void()>(16, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'onSelectTool'
        QtMocHelpers::SlotData<void()>(17, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'onResizeTool'
        QtMocHelpers::SlotData<void()>(18, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'onCropTool'
        QtMocHelpers::SlotData<void()>(19, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'onCanvasVerticalReflection'
        QtMocHelpers::SlotData<void()>(20, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'onCanvasHorizontalReflection'
        QtMocHelpers::SlotData<void()>(21, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'onCanvasYellowFilter'
        QtMocHelpers::SlotData<void()>(22, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'onCanvasPurpleFilter'
        QtMocHelpers::SlotData<void()>(23, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'onCanvasInfraRedFilter'
        QtMocHelpers::SlotData<void()>(24, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'onCanvasUndo'
        QtMocHelpers::SlotData<void()>(25, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'onCanvasRedo'
        QtMocHelpers::SlotData<void()>(26, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'onCanvasReset'
        QtMocHelpers::SlotData<void()>(27, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'onApplyBnWFilter'
        QtMocHelpers::SlotData<void()>(28, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'onPreviewBnWFilter'
        QtMocHelpers::SlotData<void(int)>(29, 2, QMC::AccessPrivate, QMetaType::Void, {{
            { QMetaType::Int, 30 },
        }}),
        // Slot 'onApplyBlurFilter'
        QtMocHelpers::SlotData<void()>(31, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'onPreviewBlurFilter'
        QtMocHelpers::SlotData<void(int)>(32, 2, QMC::AccessPrivate, QMetaType::Void, {{
            { QMetaType::Int, 33 },
        }}),
        // Slot 'onApplyLightOrDarkFilter'
        QtMocHelpers::SlotData<void()>(34, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'onPreviewLightOrDarkFilter'
        QtMocHelpers::SlotData<void(int)>(35, 2, QMC::AccessPrivate, QMetaType::Void, {{
            { QMetaType::Int, 36 },
        }}),
        // Slot 'onApplyOilPaintFilter'
        QtMocHelpers::SlotData<void()>(37, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'onPreviewOilPaintFilter'
        QtMocHelpers::SlotData<void(int)>(38, 2, QMC::AccessPrivate, QMetaType::Void, {{
            { QMetaType::Int, 33 },
        }}),
        // Slot 'onApplySkewFilter'
        QtMocHelpers::SlotData<void()>(39, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'onPreviewSkewFilter'
        QtMocHelpers::SlotData<void(double)>(40, 2, QMC::AccessPrivate, QMetaType::Void, {{
            { QMetaType::Double, 41 },
        }}),
    };
    QtMocHelpers::UintData qt_properties {
    };
    QtMocHelpers::UintData qt_enums {
    };
    return QtMocHelpers::metaObjectData<MainWindow, qt_meta_tag_ZN10MainWindowE_t>(QMC::MetaObjectFlag{}, qt_stringData,
            qt_methods, qt_properties, qt_enums);
}
Q_CONSTINIT const QMetaObject MainWindow::staticMetaObject = { {
    QMetaObject::SuperData::link<QMainWindow::staticMetaObject>(),
    qt_staticMetaObjectStaticContent<qt_meta_tag_ZN10MainWindowE_t>.stringdata,
    qt_staticMetaObjectStaticContent<qt_meta_tag_ZN10MainWindowE_t>.data,
    qt_static_metacall,
    nullptr,
    qt_staticMetaObjectRelocatingContent<qt_meta_tag_ZN10MainWindowE_t>.metaTypes,
    nullptr
} };

void MainWindow::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    auto *_t = static_cast<MainWindow *>(_o);
    if (_c == QMetaObject::InvokeMetaMethod) {
        switch (_id) {
        case 0: _t->applyMorphFilter((*reinterpret_cast< std::add_pointer_t<double>>(_a[1]))); break;
        case 1: _t->applyMorphAnimated((*reinterpret_cast< std::add_pointer_t<int>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<double>>(_a[2]))); break;
        case 2: _t->applyMergeToCanvas(); break;
        case 3: _t->openImage(); break;
        case 4: _t->saveImage(); break;
        case 5: _t->showApplyCancelButtons(); break;
        case 6: _t->hideApplyCancelButtons(); break;
        case 7: _t->exitApp(); break;
        case 8: _t->onLoadImage(); break;
        case 9: _t->onSaveImage(); break;
        case 10: _t->onResetImage(); break;
        case 11: _t->onUndo(); break;
        case 12: _t->onRedo(); break;
        case 13: _t->onSelectTool(); break;
        case 14: _t->onResizeTool(); break;
        case 15: _t->onCropTool(); break;
        case 16: _t->onCanvasVerticalReflection(); break;
        case 17: _t->onCanvasHorizontalReflection(); break;
        case 18: _t->onCanvasYellowFilter(); break;
        case 19: _t->onCanvasPurpleFilter(); break;
        case 20: _t->onCanvasInfraRedFilter(); break;
        case 21: _t->onCanvasUndo(); break;
        case 22: _t->onCanvasRedo(); break;
        case 23: _t->onCanvasReset(); break;
        case 24: _t->onApplyBnWFilter(); break;
        case 25: _t->onPreviewBnWFilter((*reinterpret_cast< std::add_pointer_t<int>>(_a[1]))); break;
        case 26: _t->onApplyBlurFilter(); break;
        case 27: _t->onPreviewBlurFilter((*reinterpret_cast< std::add_pointer_t<int>>(_a[1]))); break;
        case 28: _t->onApplyLightOrDarkFilter(); break;
        case 29: _t->onPreviewLightOrDarkFilter((*reinterpret_cast< std::add_pointer_t<int>>(_a[1]))); break;
        case 30: _t->onApplyOilPaintFilter(); break;
        case 31: _t->onPreviewOilPaintFilter((*reinterpret_cast< std::add_pointer_t<int>>(_a[1]))); break;
        case 32: _t->onApplySkewFilter(); break;
        case 33: _t->onPreviewSkewFilter((*reinterpret_cast< std::add_pointer_t<double>>(_a[1]))); break;
        default: ;
        }
    }
}

const QMetaObject *MainWindow::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *MainWindow::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_staticMetaObjectStaticContent<qt_meta_tag_ZN10MainWindowE_t>.strings))
        return static_cast<void*>(this);
    return QMainWindow::qt_metacast(_clname);
}

int MainWindow::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QMainWindow::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 34)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 34;
    }
    if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 34)
            *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType();
        _id -= 34;
    }
    return _id;
}
QT_WARNING_POP
