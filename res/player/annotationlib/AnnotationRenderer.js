class NoteAnnotation {
    static get defaultAppearanceAttributes() {
        return {
            bgColor: 16777215,
            bgOpacity: .8,
            fgColor: 0,
            textSize: 3.15
        }
    }
    constructor(t, e) {
        if (!t) throw new Error("Annotation data must be provided");
        this.data = t, this.closeElement = e, this.element = document.createElement("div"), this.element.classList.add("__cxt-ar-annotation__"), this.element.__annotationData = this.data, this.data.text && (this.textElement = document.createElement("span"), this.textElement.textContent = this.data.text, this.element.append(this.textElement)), this.setupAppearance(), "speech" !== this.data.style && "title" !== this.data.style && this.setupHoverAppearance(), this.element.setAttribute("data-ar-type", this.data.type), this.data.style && this.element.setAttribute("data-ar-style", this.data.style), this.element.setAttribute("hidden", ""), this.textScaling = 100, this.paddingMultiplier = 1, this.closeButtonScaling = .0423, this.closeButtonOffset = -1.8
    }
    setupAppearance() {
        let t = this.constructor.defaultAppearanceAttributes;
        isNaN(this.data.textSize) || (t.textSize = this.data.textSize), isNaN(this.data.fgColor) || (t.fgColor = this.data.fgColor), isNaN(this.data.bgColor) || (t.bgColor = this.data.bgColor), isNaN(this.data.bgOpacity) || (t.bgOpacity = this.data.bgOpacity), this.data.bgColor = t.bgColor, this.data.bgOpacity = t.bgOpacity, this.data.fgColor = t.fgColor, this.data.textSize = t.textSize, this.element.style.color = "#" + decimalToHex(t.fgColor)
    }
    setupHoverAppearance() {
        const {
            bgOpacity: t,
            bgColor: e
        } = this.data, s = getFinalAnnotationColor(t, e);
        this.element.style.backgroundColor = s, this.element.addEventListener("mouseenter", (() => {
            this.element.style.backgroundColor = getFinalAnnotationColor(t, e, !0), this.closeElement.currentAnnotation = this, this.closeElement.lastAnnotation = this, this.closeElement.style.display = "block";
            const s = this.closeButtonSize / 2,
                i = this.element.getBoundingClientRect();
            this.closeElement.style.left = i.right - s + "px", this.closeElement.style.top = i.top - s + "px"
        })), this.element.addEventListener("mouseleave", (() => {
            this.element.style.backgroundColor = getFinalAnnotationColor(t, e, !1), this.closeElement.currentAnnotation = null, setTimeout((() => {
                this.closeElement.hovered || null !== this.closeElement.currentAnnotation || (this.closeElement.style.display = "none")
            }), 100)
        })), "url" === this.data.actionType && (this.element.style.cursor = "pointer")
    }
    createCloseElement(t = 10) {
        const e = document.createElementNS("http://www.w3.org/2000/svg", "svg");
        e.setAttribute("viewBox", "0 0 100 100"), e.classList.add("__cxt-ar-annotation-close__");
        const s = document.createElementNS(e.namespaceURI, "path");
        s.setAttribute("d", "M25 25 L 75 75 M 75 25 L 25 75"), s.setAttribute("stroke", "#bbb"), s.setAttribute("stroke-width", t), s.setAttribute("x", 5), s.setAttribute("y", 5);
        const i = document.createElementNS(e.namespaceURI, "circle");
        return i.setAttribute("cx", 50), i.setAttribute("cy", 50), i.setAttribute("r", 50), e.append(i, s), e
    }
    show() {
        this.element.removeAttribute("hidden")
    }
    hide() {
        this.element.setAttribute("hidden", "")
    }
    updateTextSize(t) {
        if (this.data.textSize) {
            const e = this.data.textSize / this.textScaling * t;
            this.fontSize = `${e}px`
        }
    }
    updateCloseSize(t) {
        const e = t * this.closeButtonScaling;
        this.closeElement.style.width = e + "px", this.closeElement.style.height = e + "px", this.closeElement.style.right = e / this.closeButtonOffset + "px", this.closeElement.style.top = e / this.closeButtonOffset + "px", this.closeButtonSize = e
    }
    setDimensions(t, e, s, i) {
        this.left = t, this.top = e, this.width = s, this.height = i
    }
    setPadding(t, e) {
        t = t * this.paddingMultiplier + "px", e = e * this.paddingMultiplier + "px", this.element.style.padding = `${e} ${t} ${e} ${t}`
    }
    get closed() {
        return this.element.hasAttribute("data-ar-closed")
    }
    get hidden() {
        return this.element.hasAttribute("hidden")
    }
    get type() {
        return this.data.type
    }
    get style() {
        return this.data.style
    }
    set left(t) {
        this.element.style.left = t
    }
    set top(t) {
        this.element.style.top = t
    }
    set width(t) {
        this.element.style.width = t
    }
    set height(t) {
        this.element.style.height = t
    }
    set fontSize(t) {
        this.element.style.fontSize = t
    }
}

function decimalToHex(t) {
    let e = t.toString(16);
    return e = "000000".substr(0, 6 - e.length) + e, e
}

function getFinalAnnotationColor(t, e, s = !1, i = 230) {
    if (!isNaN(t) && !isNaN(e)) {
        const n = s ? i.toString(16) : Math.floor(255 * t).toString(16);
        return `#${decimalToHex(e)}${n}`
    }
    return "#FFFFFFFF"
}
class SpeechAnnotation extends NoteAnnotation {
    static get horizontalBaseStartMultiplier() {
        return .17379070765180116
    }
    static get horizontalBaseEndMultiplier() {
        return .14896346370154384
    }
    static get verticalBaseStartMultiplier() {
        return .12
    }
    static get verticalBaseEndMultiplier() {
        return .3
    }
    constructor(t, e) {
        super(t, e), this.createSpeechBubble(), this.setupHoverAppearance(), this.textX = 0, this.textY = 0, this.textWidth = 0, this.textHeight = 0, this.baseStartX = 0, this.baseStartY = 0, this.baseEndX = 0, this.baseEndY = 0, this.pointX = 0, this.pointY = 0, this.directionPadding = 20, this.paddingMultiplier = 2
    }
    getPointDirection(t, e, s, i, n, o, a = 20) {
        return n > t + s - s / 2 && o > e + i ? "br" : n < t + s - s / 2 && o > e + i ? "bl" : n > t + s - s / 2 && o < e - a ? "tr" : n < t + s - s / 2 && o < e ? "tl" : n > t + s && o > e - a && o < e + i - a ? "r" : n < t && o > e && o < e + i ? "l" : void 0
    }
    createSpeechBubble(t = "white") {
        this.speechSvg = document.createElementNS("http://www.w3.org/2000/svg", "svg"), this.speechSvg.classList.add("__cxt-ar-annotation-speech-bubble__"), this.speechSvg.style.position = "absolute", this.speechSvg.setAttribute("width", "100%"), this.speechSvg.setAttribute("height", "100%"), this.speechSvg.style.left = "0", this.speechSvg.style.left = "0", this.speechSvg.style.display = "block", this.speechSvg.style.overflow = "visible", this.speechTriangle = document.createElementNS("http://www.w3.org/2000/svg", "path"), this.speechTriangle.setAttribute("fill", t), this.speechSvg.append(this.speechTriangle);
        const {
            bgOpacity: e,
            bgColor: s
        } = this.data;
        this.speechTriangle.setAttribute("fill", getFinalAnnotationColor(e, s, !1)), this.element.prepend(this.speechSvg)
    }
    updateSpeechBubble(t, e, s, i, n, o) {
        const h = this.constructor.horizontalBaseStartMultiplier,
            r = this.constructor.horizontalBaseEndMultiplier,
            c = this.constructor.verticalBaseStartMultiplier,
            d = this.constructor.verticalBaseEndMultiplier,
            p = this.getPointDirection(t, e, s, i, n, o, this.directionPadding);
        let u = "";
        if ("br" === p) this.baseStartX = s - s * h * 2, this.baseEndX = this.baseStartX + s * r, this.baseStartY = i, this.baseEndY = i, this.pointX = n - t, this.pointY = o - e, this.height = o - e + "px", u = `L${s} ${i} L${s} 0 L0 0 L0 ${this.baseStartY} L${this.baseStartX} ${this.baseStartY}`, this.textWidth = s, this.textHeight = i, this.textX = 0, this.textY = 0;
        else if ("bl" === p) this.baseStartX = s * h, this.baseEndX = this.baseStartX + s * r, this.baseStartY = i, this.baseEndY = i, this.pointX = n - t, this.pointY = o - e, this.height = o - e + "px", u = `L${s} ${i} L${s} 0 L0 0 L0 ${this.baseStartY} L${this.baseStartX} ${this.baseStartY}`, this.textWidth = s, this.textHeight = i, this.textX = 0, this.textY = 0;
        else if ("tr" === p) {
            this.baseStartX = s - s * h * 2, this.baseEndX = this.baseStartX + s * r;
            const a = e - o;
            this.baseStartY = a, this.baseEndY = a, this.top = e - a + "px", this.height = i + a + "px", this.pointX = n - t, this.pointY = 0, u = `L${s} ${a} L${s} ${i+a} L0 ${i+a} L0 ${a} L${this.baseStartX} ${this.baseStartY}`, this.textWidth = s, this.textHeight = i, this.textX = 0, this.textY = a
        } else if ("tl" === p) {
            this.baseStartX = s * h, this.baseEndX = this.baseStartX + s * r;
            const a = e - o;
            this.baseStartY = a, this.baseEndY = a, this.top = e - a + "px", this.height = i + a + "px", this.pointX = n - t, this.pointY = 0, u = `L${s} ${a} L${s} ${i+a} L0 ${i+a} L0 ${a} L${this.baseStartX} ${this.baseStartY}`, this.textWidth = s, this.textHeight = i, this.textX = 0, this.textY = a
        } else if ("r" === p) {
            const a = n - (t + s);
            this.baseStartX = s, this.baseEndX = s, this.width = s + a + "px", this.baseStartY = i * c, this.baseEndY = this.baseStartY + i * d, this.pointX = s + a, this.pointY = o - e, u = `L${this.baseStartX} ${i} L0 ${i} L0 0 L${this.baseStartX} 0 L${this.baseStartX} ${this.baseStartY}`, this.textWidth = s, this.textHeight = i, this.textX = 0, this.textY = 0
        } else if ("l" === p) {
            const a = t - n;
            this.baseStartX = a, this.baseEndX = a, this.left = t - a + "px", this.width = s + a + "px", this.baseStartY = i * c, this.baseEndY = this.baseStartY + i * d, this.pointX = 0, this.pointY = o - e, u = `L${this.baseStartX} ${i} L${s+this.baseStartX} ${i} L${s+this.baseStartX} 0 L${this.baseStartX} 0 L${this.baseStartX} ${this.baseStartY}`, this.textWidth = s, this.textHeight = i, this.textX = a, this.textY = 0
        }
        this.textElement && (this.textElement.style.left = this.textX + "px", this.textElement.style.top = this.textY + "px", this.textElement.style.width = this.textWidth + "px", this.textElement.style.height = this.textHeight + "px");
        const m = `M${this.baseStartX} ${this.baseStartY} L${this.pointX} ${this.pointY} L${this.baseEndX} ${this.baseEndY} ${u}`;
        this.speechTriangle.setAttribute("d", m)
    }
    updateCloseSize(t) {
        const e = t * this.closeButtonScaling;
        this.closeElement.style.width = e + "px", this.closeElement.style.height = e + "px", this.closeButtonSize = e
    }
    setupHoverAppearance() {
        const {
            bgOpacity: t,
            bgColor: e
        } = this.data;
        this.speechTriangle.addEventListener("mouseover", (() => {
            this.closeElement.currentAnnotation = this, this.closeElement.lastAnnotation = this;
            const s = this.textX + this.textWidth + this.closeButtonSize / this.closeButtonOffset,
                i = this.textY + this.closeButtonSize / this.closeButtonOffset,
                n = this.element.getBoundingClientRect();
            this.closeElement.style.left = n.left + s + "px", this.closeElement.style.top = n.top + i + "px", this.closeElement.style.display = "block", this.speechTriangle.setAttribute("fill", getFinalAnnotationColor(t, e, !0))
        })), this.speechTriangle.addEventListener("mouseout", (() => {
            this.closeElement.currentAnnotation = null, setTimeout((() => {
                this.closeElement.hovered || null !== this.closeElement.currentAnnotation || (this.closeElement.style.display = "none", this.speechTriangle.setAttribute("fill", getFinalAnnotationColor(t, e, !1)))
            }), 100)
        })), "url" === this.data.actionType && (this.element.style.cursor = "pointer")
    }
    setPadding(t, e) {
        t = t * this.paddingMultiplier + "px", e = e * this.paddingMultiplier + "px", this.textElement && (this.textElement.style.padding = `${e} ${t} ${e} ${t}`)
    }
}
class HighlightAnnotation extends NoteAnnotation {
    constructor(t, e) {
        super(t, e);
        const {
            bgOpacity: s
        } = this.data;
        this.element.style.backgroundColor = "", this.element.style.border = `2.5px solid ${getFinalAnnotationColor(s,8748933,!1)}`
    }
    setupHoverAppearance() {
        const {
            bgOpacity: t,
            bgColor: e,
            actionType: s
        } = this.data;
        this.element.addEventListener("mouseenter", (() => {
            this.closeElement.currentAnnotation = this, this.closeElement.lastAnnotation = this, this.element.style.border = `2.5px solid ${getFinalAnnotationColor(t,e,!0)}`, this.closeElement.style.display = "block";
            const s = this.closeButtonSize / 2,
                i = this.element.getBoundingClientRect();
            this.closeElement.style.left = i.right - s + "px", this.closeElement.style.top = i.top - s + "px"
        })), this.element.addEventListener("mouseleave", (() => {
            this.element.style.border = `2.5px solid ${getFinalAnnotationColor(t,8748933,!1)}`, this.closeElement.currentAnnotation = null, setTimeout((() => {
                this.closeElement.hovered || null !== this.closeElement.currentAnnotation || (this.closeElement.style.display = "none")
            }), 100)
        })), "url" === s && (this.element.style.cursor = "pointer")
    }
}
class HighlightTextAnnotation extends NoteAnnotation {
    constructor(t, e, s) {
        t.x += s.data.x, t.y += s.data.y, super(t, e), this.element.style.backgroundColor = "", this.element.style.border = "", this.element.style.pointerEvents = "none", s.element.addEventListener("mouseenter", (() => {
            this.show()
        })), s.element.addEventListener("mouseleave", (() => {
            this.hide()
        })), this.closeElement.style.display = "none", this.closeElement.style.cursor = "default"
    }
    setupHoverAppearance() {}
}
class AnnotationRenderer {
    constructor(t, e, s, i = 200) {
        if (!t) throw new Error("Annotation objects must be provided");
        if (!e) throw new Error("An element to contain the annotations must be provided");
        s && s.getVideoTime && s.seekTo ? this.playerOptions = s : console.info("AnnotationRenderer is running without a player. The update method will need to be called manually."), this.annotations = [], this.container = e, this.annotationsContainer = document.createElement("div"), this.annotationsContainer.classList.add("__cxt-ar-annotations-container__"), this.annotationsContainer.setAttribute("data-layer", "4"), this.annotationsContainer.addEventListener("click", (t => {
            this.annotationClickHandler(t)
        })), this.closeElement = this.createCloseElement(), this.closeElement.style.cursor = "pointer", this.closeElement.addEventListener("click", (() => {
            const t = this.closeElement.lastAnnotation;
            t.element.setAttribute("data-ar-closed", ""), t.element.setAttribute("hidden", ""), this.closeElement.style.display = "none", this.closeElement.style.cursor = "default"
        })), this.closeElement.addEventListener("mouseenter", (() => {
            this.closeElement.hovered = !0
        })), this.closeElement.addEventListener("mouseleave", (() => {
            const t = this.closeElement.lastAnnotation;
            if (this.closeElement.style.display = "none", t && t.speechTriangle) {
                const {
                    bgOpacity: e,
                    bgColor: s
                } = t.data;
                t.speechTriangle.style.cursor = "default", t.speechTriangle.setAttribute("fill", getFinalAnnotationColor(e, s, !1))
            }
            this.closeElement.hovered = !1
        })), document.body.append(this.closeElement), this.container.prepend(this.annotationsContainer), this.createAnnotationElements(t), this.updateAllAnnotationSizes(), window.addEventListener("DOMContentLoaded", (() => {
            this.updateAllAnnotationSizes()
        })), this.updateInterval = i, this.updateIntervalId = null
    }
    changeAnnotationData(t) {
        this.stop(), this.removeAnnotationElements(), this.annotations = t, this.createAnnotationElements(), this.start()
    }
    createAnnotationElements(t) {
        const e = {},
            s = {};
        for (const i of t) {
            let t;
            "speech" === i.style ? t = new SpeechAnnotation(i, this.closeElement) : "highlight" === i.type ? (t = new HighlightAnnotation(i, this.closeElement), e[i.id] = t) : "highlightText" === i.style ? s[i.highlightId] = i : t = new NoteAnnotation(i, this.closeElement), t && (this.annotations.push(t), this.annotationsContainer.append(t.element))
        }
        for (const t in e) {
            const i = s[t];
            if (i) {
                const s = e[t],
                    n = new HighlightTextAnnotation(i, this.closeElement, s);
                this.annotations.push(n), this.annotationsContainer.append(n.element)
            }
        }
    }
    createCloseElement() {
        const t = document.createElementNS("http://www.w3.org/2000/svg", "svg");
        t.setAttribute("viewBox", "0 0 100 100"), t.classList.add("__cxt-ar-annotation-close__");
        const e = document.createElementNS(t.namespaceURI, "path");
        e.setAttribute("d", "M25 25 L 75 75 M 75 25 L 25 75"), e.setAttribute("stroke", "#bbb"), e.setAttribute("stroke-width", 10), e.setAttribute("x", 5), e.setAttribute("y", 5);
        const s = document.createElementNS(t.namespaceURI, "circle");
        return s.setAttribute("cx", 50), s.setAttribute("cy", 50), s.setAttribute("r", 50), t.append(s, e), t
    }
    removeAnnotationElements() {
        for (const t of this.annotations) t.element.remove()
    }
    update(t) {
        for (const e of this.annotations) {
            if (e.closed || "highlightText" === e.style) continue;
            const s = e.data.timeStart,
                i = e.data.timeEnd;
            e.hidden && t >= s && t < i ? e.show() : !e.hidden && (t < s || t > i) && e.hide()
        }
    }
    start() {
        if (!this.playerOptions) throw new Error("playerOptions must be provided to use the start method");
        const t = this.playerOptions.getVideoTime();
        this.updateIntervalId || (this.update(t), this.updateIntervalId = setInterval((() => {
            const t = this.playerOptions.getVideoTime();
            this.update(t), window.dispatchEvent(new CustomEvent("__ar_renderer_start"))
        }), this.updateInterval))
    }
    stop() {
        if (!this.playerOptions) throw new Error("playerOptions must be provided to use the stop method");
        const t = this.playerOptions.getVideoTime();
        this.updateIntervalId && (this.update(t), clearInterval(this.updateIntervalId), this.updateIntervalId = null, window.dispatchEvent(new CustomEvent("__ar_renderer_stop")))
    }
    updateAnnotationDimensions(t, e, s) {
        const i = this.container.getBoundingClientRect().width,
            n = this.container.getBoundingClientRect().height,
            o = i / e,
            a = n / s;
        let l = i,
            h = n;
        o % 1 == 0 && a % 1 == 0 || (o > a ? (l = n / s * e, h = n) : a > o && (l = i, h = i / e * s));
        const r = (i - l) / 2 / i * 100,
            c = (n - h) / 2 / n * 100,
            d = l / i,
            p = h / n;
        for (const e of t) {
            let t = r + e.data.x * d,
                s = c + e.data.y * p,
                o = e.data.width * d,
                a = e.data.height * p;
            e.setDimensions(`${t}%`, `${s}%`, `${o}%`, `${a}%`);
            let u = .008 * l,
                m = .008 * h;
            if (e.setPadding(u, m), "speech" === e.style && e.speechSvg) {
                const l = this.percentToPixels(i, t),
                    h = this.percentToPixels(n, s),
                    u = this.percentToPixels(i, o),
                    m = this.percentToPixels(n, a);
                let g = r + e.data.sx * d,
                    b = c + e.data.sy * p;
                g = this.percentToPixels(i, g), b = this.percentToPixels(n, b), e.updateSpeechBubble(l, h, u, m, g, b)
            }
            e.updateTextSize(h), e.updateCloseSize(h)
        }
    }
    updateAllAnnotationSizes() {
        if (this.playerOptions && this.playerOptions.getOriginalVideoWidth && this.playerOptions.getOriginalVideoHeight) {
            const t = this.playerOptions.getOriginalVideoWidth(),
                e = this.playerOptions.getOriginalVideoHeight();
            this.updateAnnotationDimensions(this.annotations, t, e)
        } else {
            const t = this.container.getBoundingClientRect().width,
                e = this.container.getBoundingClientRect().height;
            this.updateAnnotationDimensions(this.annotations, t, e)
        }
    }
    hideAll() {
        for (const t of this.annotations) t.hide()
    }
    annotationClickHandler(t) {
        let e = t.target;
        if (!e.matches(".__cxt-ar-annotation__") && !e.closest(".__cxt-ar-annotation-close__") && (e = e.closest(".__cxt-ar-annotation__"), !e)) return null;
        let s = e.__annotationData;
        if (e && s)
            if ("time" === s.actionType) {
                const t = s.actionSeconds;
                if (this.playerOptions) {
                    this.playerOptions.seekTo(t);
                    const e = this.playerOptions.getVideoTime();
                    this.update(e)
                }
                window.dispatchEvent(new CustomEvent("__ar_seek_to", {
                    detail: {
                        seconds: t
                    }
                }))
            } else if ("url" === s.actionType) {
            const t = {
                    url: s.actionUrl,
                    target: s.actionUrlTarget || "current"
                },
                e = this.extractTimeHash(new URL(t.url));
            e && e.hasOwnProperty("seconds") && (t.seconds = e.seconds), window.dispatchEvent(new CustomEvent("__ar_annotation_click", {
                detail: t
            }))
        }
    }
    setUpdateInterval(t) {
        this.updateInterval = t, this.stop(), this.start()
    }
    extractTimeHash(t) {
        if (!t) throw new Error("A URL must be provided");
        const e = t.hash;
        if (e && e.startsWith("#t=")) {
            const e = t.hash.split("#t=")[1];
            return {
                seconds: this.timeStringToSeconds(e)
            }
        }
        return !1
    }
    timeStringToSeconds(t) {
        let e = 0;
        const s = t.split("h"),
            i = (s[1] || t).split("m"),
            n = (i[1] || t).split("s");
        return s[0] && 2 === s.length && (e += 60 * parseInt(s[0], 10) * 60), i[0] && 2 === i.length && (e += 60 * parseInt(i[0], 10)), n[0] && 2 === n.length && (e += parseInt(n[0], 10)), e
    }
    percentToPixels(t, e) {
        return t * e / 100
    }
}
window.AnnotationRenderer = AnnotationRenderer;
