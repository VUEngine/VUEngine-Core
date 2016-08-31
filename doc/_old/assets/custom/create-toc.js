var $currentNav = null;
var $toc = $('#jae-sidebar-toc');
$("h2,h3,h4,h5,h6", ".documentation").each(function() {
    var $this = $(this);
    var currentLabel = $this.html();

    var newId = currentLabel.toLowerCase();
    newId = replaceAll(' ', '-', newId);
    newId = replaceAll("'", '', newId);
    newId = replaceAll('&amp;', 'and', newId);
    newId = replaceAll('&', 'and', newId);

    $this.attr('id', newId);

    var $newLi = $('<li/>');
    var $newLiA = $('<a/>', {
        href: '#' + newId,
        text: currentLabel
    });
    $toc.append($newLi);
    $newLi.append($newLiA);

    if ($this.prop("tagName") === 'H2') {
        var $newUl = $('<ul/>', {
            class: 'nav'
        });
        $newLi.append($newUl);
        $currentNav = $newUl;
    } else {
        $currentNav.append($newLi);
    }
});
function replaceAll(find, replace, str) {
    return str.replace(new RegExp(find, 'g'), replace);
}
