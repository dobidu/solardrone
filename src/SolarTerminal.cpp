#include "SolarTerminal.h"
#include <cmath>

SolarTerminal::SolarTerminal() {
    startTimer(500);
    addLog("SOLARDRONE v2.8 -- NOAA SWPC");
    addLog("awaiting solar data...");
}

void SolarTerminal::timerCallback() {
    cursorOn = !cursorOn;
    repaint();
}

juce::String SolarTerminal::timeTag() const {
    auto t = juce::Time::getCurrentTime();
    return "[" + juce::String(t.getHours()).paddedLeft('0', 2) + ":"
               + juce::String(t.getMinutes()).paddedLeft('0', 2) + ":"
               + juce::String(t.getSeconds()).paddedLeft('0', 2) + "]";
}

void SolarTerminal::addLog(const juce::String& text, bool alert) {
    if (logCount < kLogMax) {
        log[(logHead + logCount) % kLogMax] = {text, alert};
        ++logCount;
    } else {
        log[logHead] = {text, alert};
        logHead = (logHead + 1) % kLogMax;
    }
}

void SolarTerminal::updateFromSolarData(const SpaceWeatherState& sw,
                                         bool mo, bool fo, bool so) {
    cached = sw;
    modalOn = mo; fdnOn = fo; stringsOn = so;
    hasData = true;

    // Source change
    if (sw.source != prevSource) {
        const char* src = (sw.source == SpaceWeatherState::Source::live) ? "live"
                        : (sw.source == SpaceWeatherState::Source::cached) ? "cached" : "default";
        addLog(timeTag() + " src=" + src);
        prevSource = sw.source;
    }
    // Kp jump ≥1
    if (prevKp > -90.f && std::abs(sw.kp - prevKp) >= 1.0f) {
        bool alert = sw.kp >= 6.f;
        juce::String tag = sw.kp >= 6.f ? " STORM" : sw.kp >= 3.f ? " watch" : "";
        addLog(timeTag() + " kp=" + juce::String(sw.kp, 1) + tag, alert);
    }
    prevKp = sw.kp;

    // Dst storm onset
    if (sw.dst_index < -50.f && prevDst >= -50.f)
        addLog(timeTag() + " dst=" + juce::String((int)sw.dst_index) + " nT STORM", true);
    prevDst = sw.dst_index;

    // Proton event
    if (sw.proton_flux_10mev > 10.f && prevProton <= 10.f)
        addLog(timeTag() + " SEP prt=" + juce::String(sw.proton_flux_10mev, 1) + " pfu", true);
    prevProton = sw.proton_flux_10mev;

    repaint();
}

void SolarTerminal::paint(juce::Graphics& g) {
    const int W = getWidth(), H = getHeight();

    // Background + border
    g.setColour(juce::Colour(0xff030c0e));
    g.fillRoundedRectangle(0.f, 0.f, (float)W, (float)H, 3.f);
    g.setColour(juce::Colour(0xff0d2a1a));
    g.drawRoundedRectangle(0.5f, 0.5f, (float)(W-1), (float)(H-1), 3.f, 0.7f);

    // Scanlines
    g.setColour(juce::Colour(0xff000000).withAlpha(0.10f));
    for (int y = 0; y < H; y += 2)
        g.fillRect(0, y, W, 1);

    const juce::Font mono(juce::Font::getDefaultMonospacedFontName(), 10.0f, juce::Font::plain);
    g.setFont(mono);
    const int lh = 13, pad = 6;
    int y = 5;

    const juce::Colour cGreen = juce::Colour(0xff00e870);
    const juce::Colour cDim   = juce::Colour(0xff0f3320);
    const juce::Colour cInfo  = juce::Colour(0xff1a8844);
    const juce::Colour cAmber = juce::Colour(0xffffaa00);
    const juce::Colour cAlert = juce::Colour(0xffff4422);

    auto row = [&](const juce::String& text, juce::Colour col) {
        g.setColour(col);
        g.drawText(text, pad, y, W - pad * 2, lh, juce::Justification::left, false);
        y += lh + 1;
    };

    // ── Header ───────────────────────────────────────────────────────────────
    row("> SOLARDRONE v2.8  NOAA SWPC", cDim.brighter(0.3f));

    if (!hasData) {
        row("> awaiting solar data...", cDim);
    } else {
        const char* srcStr =
            cached.source == SpaceWeatherState::Source::live    ? "live"    :
            cached.source == SpaceWeatherState::Source::cached  ? "cached"  : "default";
        bool isLive = cached.source == SpaceWeatherState::Source::live;
        row(juce::String("> src:") + srcStr + "  age:" + juce::String(cached.data_age_s) + "s",
            isLive ? cInfo : cAmber);

        // Solar wind
        row(juce::String("> vel:") + juce::String((int)cached.velocity).paddedLeft(' ', 5)
            + " km/s  den:" + juce::String(cached.density, 1) + " /cc",
            cGreen.withAlpha(0.9f));

        juce::Colour bzCol = cached.bz_gsm < -10.f ? cAmber : cGreen.withAlpha(0.9f);
        juce::Colour kpCol = cached.kp >= 6.f ? cAlert : cached.kp >= 3.f ? cAmber : cGreen.withAlpha(0.9f);
        // bz and kp on same line, pick worst color
        juce::Colour bzkpCol = cached.kp >= 6.f ? cAlert : cached.bz_gsm < -10.f ? cAmber : cGreen.withAlpha(0.9f);
        row(juce::String("> bz :") + juce::String(cached.bz_gsm, 1).paddedLeft(' ', 6)
            + " nT   kp :" + juce::String(cached.kp, 1), bzkpCol);
        juce::ignoreUnused(bzCol, kpCol);

        juce::Colour dstCol = cached.dst_index < -50.f ? cAlert : cached.dst_index < -20.f ? cAmber : cGreen.withAlpha(0.9f);
        row(juce::String("> dst:") + juce::String((int)cached.dst_index).paddedLeft(' ', 5)
            + " nT   tmp:" + juce::String(cached.temperature / 1000.f, 0) + " kK", dstCol);

        const char* flareStr =
            cached.flare_class == SpaceWeatherState::FlareClass::X ? "X  " :
            cached.flare_class == SpaceWeatherState::FlareClass::M ? "M  " :
            cached.flare_class == SpaceWeatherState::FlareClass::C ? "C  " :
            cached.flare_class == SpaceWeatherState::FlareClass::B ? "B  " : "---";
        juce::Colour xrCol = cached.flare_class >= SpaceWeatherState::FlareClass::M ? cAlert :
                             cached.flare_class == SpaceWeatherState::FlareClass::C ? cAmber :
                             cGreen.withAlpha(0.9f);
        juce::Colour prtCol = cached.proton_flux_10mev > 10.f ? cAlert : cGreen.withAlpha(0.9f);
        juce::Colour xrprtCol = (xrCol == cAlert || prtCol == cAlert) ? cAlert :
                                (xrCol == cAmber || prtCol == cAmber) ? cAmber : cGreen.withAlpha(0.9f);
        row(juce::String("> xry:") + flareStr
            + "         prt:" + juce::String(cached.proton_flux_10mev, 1) + " pfu", xrprtCol);

        // Resonator state
        y += 2;
        auto resRow = [&](const char* name, bool on) {
            row(juce::String("> [") + name + "] " + (on ? "ON " : "OFF"),
                on ? cGreen.withAlpha(0.85f) : cDim.brighter(0.1f));
        };
        resRow("MODAL", modalOn);
        resRow("FDN  ", fdnOn);
        resRow("STR  ", stringsOn);
    }

    // ── Separator ─────────────────────────────────────────────────────────────
    y += 3;
    g.setColour(cDim.brighter(0.2f));
    g.drawHorizontalLine(y, (float)pad, (float)(W - pad));
    y += 5;

    // ── Event log ─────────────────────────────────────────────────────────────
    const int logLineH = lh + 1;
    const int remaining = H - y - lh - 4;
    const int maxVisible = std::max(0, remaining / logLineH);
    const int show = std::min(logCount, maxVisible);
    const int startIdx = (logHead + (logCount - show) + kLogMax) % kLogMax;

    for (int i = 0; i < show; ++i) {
        const auto& entry = log[(startIdx + i) % kLogMax];
        row(entry.text, entry.alert ? cAlert : cInfo.brighter(0.2f));
    }

    // ── Cursor ────────────────────────────────────────────────────────────────
    if (cursorOn && y < H - 4) {
        g.setColour(cGreen);
        g.drawText("_", pad, H - lh - 4, 12, lh, juce::Justification::left, false);
    }
}
