Improved dirstamp
=================

.. code-block:: makefile

	dirstamp.lst:
		[ ! -e $@ -o -n "$(find dir/ -newer $@ -print -quit)" ] && touch $@ || :;
