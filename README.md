Ctrl+Tab Plugin for Geany
=========================

About
-----------

Ctrl-Tab is a plugin for Geany that provides a dialog for rapid switching to last used document(s).

The Overview plugin is a small zoomed-out view next to the normal editor view
that allows to see and navigate a lot of the file at once. It is similar to
the Minimap in SublimeText or such similar feature in numerous other editors.

Screenshots
-----------

This screenshot shows with a light theme, the overview's overlay is inverted,
and the regular scrollbar is not shown. The optional tooltip can be seen
(partially cut-off) in the screenshot also. It shows the line, column and
offset information of the location of the mouse cursor.

![Overview plugin screenshot with light theme](https://github.com/leifmariposa/geany-ctrl-tab-plugin/blob/master/screenshots/screenshot.png?raw=true)

Building and Installing
-----------------------

Download the plugin https://github.com/samyBadjoudj/geany-qop-plugin

The run the following commands:

```bash
$ make
$ sudo make install
```

Using the Plugin
----------------

After having enabled the plugin inside Geany through Geany's plugin manager,
you'll need to setup a keybinding for triggering the Ctrl-Tab dialog. Go to
the preferences, and under the Keybindings tab set the Ctrl+Tab keybinding to `<Primary>Tab`. 

Using the plugin is simple. Press Ctrl+Tab and the dialog will be shown with the 
last activated document already selected, so if that is the document that you 
want release the keys and the document will be activated.
If you want to switch to any other document keep the Control key down while cycling 
through the documents with the Tab key.

The Overview uses a really simple alogrithm that
seems to work quite well. When you click on the overview, it jumps to the
corresponding location in the main editor view and scrolls it into view. When
you scroll on it, it actually just does the same thing, except it jumps to
each location really quickly as the mouse moves, simulating a scrolling
effect. If the scrolling behaviour feels a little weird at first, this is
probably why.

Scrolling the main editor view causes the overview to update it's overlay to
show which part of the file is visible in the main editor.

License
----------------

This plugin is distributed under the terms of the GNU General Public License
as published by the Free Software Foundation, either version 2 of the
License, or (at your option) any later version. You should have received a copy
of the GNU General Public License along with this plugin.  If not, see
<http://www.gnu.org/licenses/>. 

Contact
----------------

You can email me at <leifmariposa(at)hotmail(dot)com>
 
 
Bug reports and feature requests
----------------

To report a bug or ask for a new feature, please use the tracker
on GitHub: https://github.com/leifmariposa/geany-ctrl-tab-plugin/issues
