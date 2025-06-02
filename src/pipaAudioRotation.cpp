/**
 * Author: Jason White
 * License: Public Domain
 *
 * Description:
 * This is a simple test program to hook into PulseAudio volume change
 * notifications. It was created for the possibility of having an automatically
 * updating volume widget in a tiling window manager status bar.
 *
 * Compiling:
 *
 *     g++ $(shell pkg-config libpulse --cflags --libs) pulsetest.c -o pulsetest
 */
#include <stdio.h>
#include <assert.h>
#include <string.h>
#include <signal.h>
#include <pulse/pulseaudio.h>
#include <pulse/glib-mainloop.h>
#include <alsa/asoundlib.h>
#include <QRotationSensor>
#include <QDebug>
#include <QOrientationSensor>
#include <QGuiApplication>
#include <QProcess>
#include <QScreen>

class PulseAudio {
private:
    pa_glib_mainloop *_mainloop;
    pa_mainloop_api *_mainloop_api;
    pa_context *_context;
    pa_signal_event *_signal;

public:
    PulseAudio() : _mainloop(NULL), _mainloop_api(NULL), _context(NULL), _signal(NULL) {
    }

    enum Orientation {
        spin_0,
        spin_90,
        spin_180,
        spin_270,
    }  _orientation = spin_0;

    /**
     * Initializes state and connects to the PulseAudio server.
     */
    bool initialize() {
        _mainloop = pa_glib_mainloop_new(nullptr);
        if (!_mainloop) {
            fprintf(stderr, "pa_mainloop_new() failed.\n");
            return false;
        }

        _mainloop_api = pa_glib_mainloop_get_api(_mainloop);

        if (pa_signal_init(_mainloop_api) != 0) {
            fprintf(stderr, "pa_signal_init() failed\n");
            return false;
        }

        _signal = pa_signal_new(SIGINT, exit_signal_callback, this);
        if (!_signal) {
            fprintf(stderr, "pa_signal_new() failed\n");
            return false;
        }
        signal(SIGPIPE, SIG_IGN);

        _context = pa_context_new(_mainloop_api, "PulseAudio Test");
        if (!_context) {
            fprintf(stderr, "pa_context_new() failed\n");
            return false;
        }

        if (pa_context_connect(_context, NULL, PA_CONTEXT_NOAUTOSPAWN, NULL) < 0) {
            fprintf(stderr, "pa_context_connect() failed: %s\n", pa_strerror(pa_context_errno(_context)));
            return false;
        }

        pa_context_set_state_callback(_context, context_state_callback, this);

        return true;
    }

    /**
     * Exits the main loop with the specified return code.
     */
    void quit(int ret = 0) {
        QApplication::quit();
    }

    void setOrientation(Orientation orientation) {
        _orientation = orientation;

        setRotation(_orientation);
    }

    /**
     * Called when the PulseAudio system is to be destroyed.
     */
    void destroy() {
        if (_context) {
            pa_context_unref(_context);
            _context = NULL;
        }

        if (_signal) {
            pa_signal_free(_signal);
            pa_signal_done();
            _signal = NULL;
        }

        if (_mainloop) {
            pa_glib_mainloop_free(_mainloop);
            _mainloop = NULL;
            _mainloop_api = NULL;
        }
    }

    ~PulseAudio() {
        destroy();
    }

    static void setRotation(int rotation) {
        QProcess::execute("amixer", {"cset", "name=\"aw882xx_spin_switch\"", QString::number(rotation)});
    }

private:

    /*
     * Called on SIGINT.
     */
    static void exit_signal_callback(pa_mainloop_api *m, pa_signal_event *e, int sig, void *userdata) {
        PulseAudio *pa = (PulseAudio *) userdata;
        if (pa) pa->quit();
    }

    /*
     * Called whenever the context status changes.
     */
    static void context_state_callback(pa_context *c, void *userdata) {
        assert(c && userdata);

        PulseAudio *pa = (PulseAudio *) userdata;

        switch (pa_context_get_state(c)) {
            case PA_CONTEXT_CONNECTING:
            case PA_CONTEXT_AUTHORIZING:
            case PA_CONTEXT_SETTING_NAME:
                break;

            case PA_CONTEXT_READY:
                fprintf(stderr, "PulseAudio connection established.\n");

                // Subscribe to sink events from the server. This is how we get
                // volume change notifications from the server.
                pa_context_set_subscribe_callback(c, subscribe_callback, userdata);
                pa_context_subscribe(c, PA_SUBSCRIPTION_MASK_SINK, NULL, NULL);
                break;

            case PA_CONTEXT_TERMINATED:
                pa->quit(0);
                fprintf(stderr, "PulseAudio connection terminated.\n");
                break;

            case PA_CONTEXT_FAILED:
            default:
                fprintf(stderr, "Connection failure: %s\n", pa_strerror(pa_context_errno(c)));
                pa->quit(1);
                break;
        }
    }

    /*
     * Called when an event we subscribed to occurs.
     */
    static void subscribe_callback(pa_context *c, pa_subscription_event_type_t type, uint32_t idx, void *userdata) {
        unsigned facility = type & PA_SUBSCRIPTION_EVENT_FACILITY_MASK;

        pa_operation *op = NULL;

        switch (facility) {
            case PA_SUBSCRIPTION_EVENT_SINK:
                op = pa_context_get_sink_info_by_index(c, idx, sink_info_callback, userdata);
                break;

            default:
                assert(0); // Got event we aren't expecting.
                break;
        }

        if (op)
            pa_operation_unref(op);
    }

    /*
     * Called when the requested sink information is ready.
     */
    static void sink_info_callback(pa_context *c, const pa_sink_info *i, int eol, void *userdata) {
        PulseAudio *pa = (PulseAudio *) userdata;

        if (i && strcmp(i->name, "sink.deep_buffer") == 0 && i->state == PA_SINK_RUNNING) {
            setRotation(pa->_orientation);
        }
    }
};


int main(int argc, char *argv[]) {

    QGuiApplication a(argc, argv);

    PulseAudio pa;

    QScreen *screen = a.primaryScreen();
    screen->setOrientationUpdateMask(
        Qt::PortraitOrientation |
        Qt::LandscapeOrientation |
        Qt::InvertedPortraitOrientation |
        Qt::InvertedLandscapeOrientation
    );

    auto changeOrientation = [&](Qt::ScreenOrientation orientation){
        switch (orientation) {
        case Qt::ScreenOrientation::PrimaryOrientation:
        case Qt::ScreenOrientation::PortraitOrientation:
            pa.setOrientation(PulseAudio::spin_0);
            break;
        case Qt::ScreenOrientation::InvertedPortraitOrientation:
            pa.setOrientation(PulseAudio::spin_180);
            break;
        case Qt::ScreenOrientation::InvertedLandscapeOrientation:
            pa.setOrientation(PulseAudio::spin_90);
            break;
        case Qt::ScreenOrientation::LandscapeOrientation:
            pa.setOrientation(PulseAudio::spin_270);
            break;
        }
    };

    changeOrientation(screen->orientation());

    QObject::connect(screen, &QScreen::orientationChanged, changeOrientation);

    if (!pa.initialize())
        return 0;

    return a.exec();
}
