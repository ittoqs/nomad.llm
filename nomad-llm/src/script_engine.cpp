#include "script_engine.h"
#include <QDebug>
#undef slots
#include <pybind11/pybind11.h>
#include <pybind11/embed.h>

namespace py = pybind11;

ScriptEngine::ScriptEngine(QObject *parent) : QObject(parent) {
    py::initialize_interpreter();
    
    // Install Python sandbox audit hook for strict security
    try {
        py::exec(R"EOF(
import sys
def sandbox_audit_hook(event, args):
    # Block writing files
    if event == "open":
        mode = args[1]
        if isinstance(mode, str) and any(c in mode for c in ['w', 'a', '+', 'x']):
            raise RuntimeError(f"Sandbox security restriction: write access is denied")
    
    # Block network except localhost
    if event == "socket.connect":
        address = args[1] if len(args) > 1 else None
        if address and isinstance(address, tuple) and len(address) > 0:
            host = address[0]
            if host not in ("127.0.0.1", "localhost", "::1"):
                raise RuntimeError(f"Sandbox security restriction: network connection to {host} is denied")

    # Block OS command execution
    if event in ["os.system", "os.spawn", "os.exec", "subprocess.Popen", "os.posix_spawn"]:
        raise RuntimeError(f"Sandbox security restriction: Command execution is denied ({event})")

sys.addaudithook(sandbox_audit_hook)
)EOF");
    } catch (std::exception &e) {
        qDebug() << "Failed to install Python sandbox audit hook:" << e.what();
    }
}

ScriptEngine::~ScriptEngine() {
    py::finalize_interpreter();
}

QString ScriptEngine::executeScript(const QString &scriptCode) {
    qDebug() << "ScriptEngine: Executing script\n" << scriptCode;
    try {
        py::dict globals = py::globals();

        // Basic sandboxing: remove dangerous builtins
        if (globals.contains("__builtins__")) {
            py::object builtins = globals["__builtins__"];
            if (py::isinstance<py::dict>(builtins)) {
                py::dict bdict = builtins.cast<py::dict>();
                if (bdict.contains("__import__")) {
                    bdict["__import__"] = py::none();
                }
                if (bdict.contains("eval")) bdict["eval"] = py::none();
                if (bdict.contains("exec")) bdict["exec"] = py::none();
                if (bdict.contains("open")) bdict["open"] = py::none();
            } else if (py::hasattr(builtins, "__import__")) {
                setattr(builtins, "__import__", py::none());
                if (py::hasattr(builtins, "eval")) setattr(builtins, "eval", py::none());
                if (py::hasattr(builtins, "exec")) setattr(builtins, "exec", py::none());
                if (py::hasattr(builtins, "open")) setattr(builtins, "open", py::none());
            }
        } else {
            py::dict bdict;
            bdict["__import__"] = py::none();
            bdict["eval"] = py::none();
            bdict["exec"] = py::none();
            bdict["open"] = py::none();
            globals["__builtins__"] = bdict;
        }

        py::dict locals;
        py::exec(scriptCode.toStdString(), globals, locals);
        return "Script executed successfully";
    } catch (std::exception &e) {
        return QString("Script Error: ") + e.what();
    }
}
