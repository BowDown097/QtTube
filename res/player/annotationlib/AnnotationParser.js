class AnnotationParser {
    static get attributeMap() {
        return {
            type: "tp",
            style: "s",
            x: "x",
            y: "y",
            width: "w",
            height: "h",
            sx: "sx",
            sy: "sy",
            timeStart: "ts",
            timeEnd: "te",
            text: "t",
            actionType: "at",
            actionUrl: "au",
            actionUrlTarget: "aut",
            actionSeconds: "as",
            bgOpacity: "bgo",
            bgColor: "bgc",
            fgColor: "fgc",
            textSize: "txsz",
            highlightId: "hid"
        }
    }
    deserializeAnnotation(t) {
        const e = this.constructor.attributeMap,
            n = t.split(","),
            r = {};
        for (const t of n) {
            const [n, s] = t.split("="), i = this.getKeyByValue(e, n);
            let o = "";
            o = ["text", "actionType", "actionUrl", "actionUrlTarget", "type", "style"].includes(i) ? decodeURIComponent(s) : parseFloat(s, 10), r[i] = o
        }
        return r
    }
    serializeAnnotation(t) {
        const e = this.constructor.attributeMap;
        let n = "";
        for (const r in t) {
            const s = e[r],
                i = ["text", "actionType", "actionUrl", "actionUrlTarget"];
            if (i.includes(r) && s && t.hasOwnProperty(r)) {
                n += `${s}=${encodeURIComponent(t[r])},`
            } else !i.includes(r) && s && t.hasOwnProperty(r) && (n += `${s}=${t[r]},`)
        }
        return n.substring(0, n.length - 1)
    }
    deserializeAnnotationList(t) {
        const e = t.split(";");
        e.length = e.length - 1;
        const n = [];
        for (const t of e) n.push(this.deserializeAnnotation(t));
        return n
    }
    serializeAnnotationList(t) {
        let e = "";
        for (const n of t) e += this.serializeAnnotation(n) + ";";
        return e
    }
    xmlToDom(t) {
        return (new DOMParser).parseFromString(t, "application/xml")
    }
    getAnnotationsFromXml(t) {
        return this.xmlToDom(t).getElementsByTagName("annotation")
    }
    parseYoutubeAnnotationList(t) {
        const e = [];
        for (const n of t) {
            const t = this.parseYoutubeAnnotation(n);
            t && e.push(t)
        }
        return e
    }
    parseYoutubeAnnotation(t) {
        const e = t,
            n = this.getAttributesFromBase(e);
        if (!n.type || "pause" === n.type) return null;
        const r = this.getTextFromBase(e),
            s = this.getActionFromBase(e),
            i = this.getBackgroundShapeFromBase(e);
        if (!i) return null;
        const o = i.timeRange.start,
            a = i.timeRange.end;
        if ("highlightText" !== n.style && (isNaN(o) || isNaN(a) || null === o || null === a)) return null;
        const g = this.getAppearanceFromBase(e);
        let l = {
            type: n.type,
            x: i.x,
            y: i.y,
            width: i.width,
            height: i.height,
            timeStart: o,
            timeEnd: a
        };
        if (n.style && (l.style = n.style), r && (l.text = r), s && (l = Object.assign(s, l)), g && (l = Object.assign(g, l)), i.hasOwnProperty("sx") && (l.sx = i.sx), i.hasOwnProperty("sy") && (l.sy = i.sy), "highlight" === l.type) l.id = n.id;
        else if ("highlightText" === l.style) {
            const t = this.getTriggerFromBase(e);
            t && (l.highlightId = t), delete l.timeStart, delete l.timeEnd
        }
        return l
    }
    getBackgroundShapeFromBase(t) {
        const e = t.getElementsByTagName("movingRegion")[0];
        if (!e) return null;
        const n = e.getAttribute("type"),
            r = e.getElementsByTagName(`${n}Region`),
            s = this.extractRegionTime(r),
            i = {
                type: n,
                x: parseFloat(r[0].getAttribute("x"), 10),
                y: parseFloat(r[0].getAttribute("y"), 10),
                width: parseFloat(r[0].getAttribute("w"), 10),
                height: parseFloat(r[0].getAttribute("h"), 10),
                timeRange: s
            },
            o = r[0].getAttribute("sx"),
            a = r[0].getAttribute("sy");
        return o && (i.sx = parseFloat(o, 10)), a && (i.sy = parseFloat(a, 10)), i
    }
    getAttributesFromBase(t) {
        const e = {};
        return e.id = t.getAttribute("id"), e.type = t.getAttribute("type"), e.style = t.getAttribute("style"), e
    }
    getTextFromBase(t) {
        const e = t.getElementsByTagName("TEXT")[0];
        if (e) return e.textContent
    }
    getActionFromBase(t) {
        const e = t.getElementsByTagName("action")[0];
        if (!e) return null;
        e.getAttribute("type");
        const n = e.getElementsByTagName("url")[0];
        if (!n) return null;
        const r = n.getAttribute("target"),
            s = n.getAttribute("value");
        if (s.startsWith("https://www.youtube.com/")) {
            const t = new URL(s),
                e = t.searchParams.get("src_vid"),
                n = t.searchParams.get("v");
            return this.linkOrTimestamp(t, e, n, r)
        }
    }
    linkOrTimestamp(t, e, n, r) {
        if (e && n && e === n) {
            let e = 0;
            const n = t.hash;
            if (n && n.startsWith("#t=")) {
                const n = t.hash.split("#t=")[1];
                e = this.timeStringToSeconds(n)
            }
            return {
                actionType: "time",
                actionSeconds: e
            }
        }
        return {
            actionType: "url",
            actionUrl: t.href,
            actionUrlTarget: r
        }
    }
    getAppearanceFromBase(t) {
        const e = t.getElementsByTagName("appearance")[0];
        if (e) {
            const t = e.getAttribute("bgAlpha"),
                n = e.getAttribute("bgColor"),
                r = e.getAttribute("fgColor"),
                s = e.getAttribute("highlightFontColor"),
                i = e.getAttribute("textSize"),
                o = {};
            return t && (o.bgOpacity = parseFloat(t, 10)), n && (o.bgColor = parseInt(n, 10)), s && (o.fgColor = parseInt(s, 10)), r && (o.fgColor = parseInt(r, 10)), i && (o.textSize = parseFloat(i, 10)), o
        }
    }
    getTriggerFromBase(t) {
        t.getElementsByTagName("trigger")[0];
        return t.getElementsByTagName("condition")[0].getAttribute("ref")
    }
    extractRegionTime(t) {
        let e = t[0].getAttribute("t");
        e = this.hmsToSeconds(e);
        let n = t[t.length - 1].getAttribute("t");
        return n = this.hmsToSeconds(n), {
            start: e,
            end: n
        }
    }
    hmsToSeconds(t) {
        let e = t.split(":"),
            n = 0,
            r = 1;
        for (; e.length > 0;) n += r * parseFloat(e.pop(), 10), r *= 60;
        return n
    }
    timeStringToSeconds(t) {
        let e = 0;
        const n = t.split("h"),
            r = (n[1] || t).split("m"),
            s = (r[1] || t).split("s");
        return n[0] && 2 === n.length && (e += 60 * parseInt(n[0], 10) * 60), r[0] && 2 === r.length && (e += 60 * parseInt(r[0], 10)), s[0] && 2 === s.length && (e += parseInt(s[0], 10)), e
    }
    getKeyByValue(t, e) {
        for (const n in t)
            if (t.hasOwnProperty(n) && t[n] === e) return n
    }
}
window.AnnotationParser = AnnotationParser;
