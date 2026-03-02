#include "perfetto_helpers.h"

PERFETTO_TRACK_EVENT_STATIC_STORAGE();

void InitializePerfetto() {
    perfetto::TracingInitArgs args;
    // The backends determine where trace events are recorded.
    // kSystemBackend connects to the system traced service (e.g. on Android).
    // kInProcessBackend records locally (useful for testing/standalone), but we want system tracing.
    args.backends = perfetto::kSystemBackend;

    perfetto::Tracing::Initialize(args);
    perfetto::TrackEvent::Register();
}
