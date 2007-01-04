define(`_PIP_CTRL_ENTRY', $1)dnl
define(`_PIP_DATA_LIST', `translit(`$@', `,', `
')')dnl
define(`_PIP_DATA_ENTRY', $1=$2)dnl
define(`_PIP_WHOLE_ENTRY',
[$1]
$3)dnl
define(`_PIP_CTRL_LIST',)dnl
