var $currentNav = null;
var $toc = $('#jae-sidebar-toc');
$("h2,h3,h4,h5,h6", ".documentation").each(function() {
    var $this = $(this);
    var currentLabel = $this.html();

    var newId = currentLabel.toLowerCase();
    newId = replaceAll(' ', '-', newId);
    newId = replaceAll("'", '', newId);

    $this.attr('id', newId);

    if ($this.prop("tagName") === 'H2') {
        var $newLi = $('<li/>');
        $toc.append($newLi);
        var $newLiA = $('<a/>', {
            href: '#' + newId,
            text: currentLabel
        });
        $newLi.append($newLiA);
        var $newUl = $('<ul/>', {
            class: 'nav'
        });
        $newLi.append($newUl);
        $currentNav = $newUl;
    } else {
        var $newLi = $('<li/>');
        $toc.append($newLi);
        var $newLiA = $('<a/>', {
            href: '#' + newId,
            text: currentLabel
        });
        $newLi.append($newLiA);
        $currentNav.append($newLi);
    }
});
function replaceAll(find, replace, str) {
    return str.replace(new RegExp(find, 'g'), replace);
}
