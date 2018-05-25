Informative git prompt for zsh
==============================

A ``zsh`` prompt that displays information about the current git repository.
In particular the branch name, difference with remote branch, number of files staged, changed, etc.

(an original idea from this `blog post`_).

Examples
--------

The prompt may look like the following:

* ``(master↑3|✚1)``: on branch ``master``, ahead of remote by 3 commits, 1 file changed but not staged
* ``(status|●2)``: on branch ``status``, 2 files staged
* ``(master|✚7…)``: on branch ``master``, 7 files changed, some files untracked
* ``(master|✖2✚3)``: on branch ``master``, 2 conflicts, 3 files changed
* ``(experimental↓2↑3|✔)``: on branch ``experimental``; your branch has diverged by 3 commits, remote by 2 commits; the repository is otherwise clean
* ``(:70c2952|✔)``: not on any branch; parent commit has hash ``70c2952``; the repository is otherwise clean

Here is how it could look like when you are ahead by 4 commits, behind by 5 commits, and have 1 staged files, 1 changed but unstaged file, and some untracked files, on branch ``dev``:

.. image:: https://github.com/olivierverdier/zsh-git-prompt/raw/master/screenshot.png
        :alt: Example


.. _blog post: http://sebastiancelis.com/2009/nov/16/zsh-prompt-git-users/

Prompt Structure
----------------

By default, the general appearance of the prompt is::

        (<branch><branch tracking>|<local status>)

The symbols are as follows:

* Local Status Symbols
        :✔: repository clean
        :●n: there are ``n`` staged files
        :✖n: there are ``n`` unmerged files
        :✚n: there are ``n`` changed but *unstaged* files
        :…n: there are ``n`` untracked files
        :⚑n: there are ``n`` stashed files

* Branch Tracking Symbols
        :↑·n: ahead of remote by ``n`` commits
        :↓·n: behind remote by ``n`` commits
        :↓·m↑·n: branches diverged, other by ``m`` commits, yours by ``n`` commits

* Branch Symbol
    When the branch name starts with a colon ``:``, it means it's actually a hash, not a branch (although it should be pretty clear, unless you name your branches like hashes :-)

Install
-------

#. Clone this repository locally.

#. Source the file ``zshrc.sh`` from your ``~/.zshrc`` config file, and use the function in your prompt. For example, somewhere in ``~/.zshrc``, you should have: ::

        . path/to/zshrc.sh
        # an example prompt
        PROMPT='%B%m%~%b$(git_super_status) %# '

#. The ``git_super_status`` function returns the formatted prompt text. For customization see below.

#. Go in a git repository and test it!

Customization
-------------
-  Define the variable ``ZSH_THEME_GIT_PROMPT_NOCACHE=1`` in order to disable caching.

-  To modify the way the prompt is put together, simply redefine a new ``git_super_status``
   after sourcing the ``zshrc.sh`` file.

-  To modify the symbols/colors of the theme, simply redefine the variables at bottom of the
   the ``zshrc.sh`` after sourcing. These are the defaults: ::

      ZSH_THEME_GIT_PROMPT_PREFIX="["
      ZSH_THEME_GIT_PROMPT_SUFFIX="]"
      ZSH_THEME_GIT_PROMPT_HASH_PREFIX=":"
      ZSH_THEME_GIT_PROMPT_SEPARATOR="|"
      ZSH_THEME_GIT_PROMPT_BRANCH="%{$fg_bold[magenta]%}"
      ZSH_THEME_GIT_PROMPT_STAGED="%{$fg[red]%}%{●%G%}"
      ZSH_THEME_GIT_PROMPT_CONFLICTS="%{$fg[red]%}%{✖%G%}"
      ZSH_THEME_GIT_PROMPT_CHANGED="%{$fg[blue]%}%{✚%G%}"
      ZSH_THEME_GIT_PROMPT_BEHIND="%{↓·%2G%}"
      ZSH_THEME_GIT_PROMPT_AHEAD="%{↑·%2G%}"
      ZSH_THEME_GIT_PROMPT_STASHED="%{$fg_bold[blue]%}%{⚑%G%}"
      ZSH_THEME_GIT_PROMPT_UNTRACKED="%{$fg[cyan]%}%{…%G%}"
      ZSH_THEME_GIT_PROMPT_CLEAN="%{$fg_bold[green]%}%{✔%G%}"

For further information please inspect:

-  ``zshrc.sh`` - For all formatting and shell functionality.
-  ``gitstatus.py`` - The script that queries the repo for information.

**Enjoy!**
