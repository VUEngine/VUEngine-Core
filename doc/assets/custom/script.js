!function (a) {
    "use strict";
    a(function () {
        var b = a(window), c = a(document.body);
        c.scrollspy({target: ".sidebar"}), b.on("load", function () {
            c.scrollspy("refresh")
        }), a(".bs-docs-container [href=#]").click(function (a) {
            a.preventDefault()
        }), setTimeout(function () {
            var b = a(".sidebar");
            b.affix({
                offset: {
                    top: function () {
                        return $('.abstract').outerHeight();
                    },
                    bottom: function () {
                        return this.bottom = a("footer").outerHeight(!0)
                    }
                }
            })
        }, 100);
    })
}(jQuery);