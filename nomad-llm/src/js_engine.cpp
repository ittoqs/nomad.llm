#include "js_engine.h"
#include <QDebug>

JsEngine::JsEngine(QObject *parent) : QObject(parent) {
    // SECURITY NOTE:
    // QJSEngine does not provide file system or OS access by default,
    // so it is already sandboxed for pure JavaScript execution.
    // It is mathematically secure from arbitrary OS execution unless 
    // explicit QObject bindings are injected into the context.
}

JsEngine::~JsEngine() {
}

QString JsEngine::executeScript(const QString &scriptCode) {
    qDebug() << "JsEngine: Executing script\n" << scriptCode;
    QJSValue result = m_engine.evaluate(scriptCode);
    if (result.isError()) {
        return QString("Script Error: line %1: %2").arg(result.property("lineNumber").toInt()).arg(result.toString());
    }
    return result.toString();
}
