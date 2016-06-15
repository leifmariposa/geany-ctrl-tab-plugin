Ctrl+Tab Plugin for Geany
=========================

About
-----------

Ctrl+Tab is a plugin for Geany that provides a dialog for rapid switching to last used document(s).


Building and Installing
-----------------------

Download the plugin from https://github.com/leifmariposa/geany-ctrl-tab-plugin

Then run the following commands:

```bash
$ make
$ sudo make install
```

Using the Plugin
----------------

After having enabled the plugin inside Geany through Geany's plugin manager,
you'll need to setup a keybinding for triggering the Ctrl+Tab dialog. Go to
the preferences, and under the Keybindings tab set the Ctrl+Tab keybinding to `<Primary>Tab`. 
Restart Geany after having enabled the plugin or the plugin will not have registered the opened documents correctly.

Using the plugin is simple. Press Ctrl+Tab and the dialog will be shown, with the 
last activated document already selected, so if that is the document that you 
want release the keys and the document will be activated.
If you want to switch to any other document keep the Control key down while cycling 
through the documents with the Tab key.
You can also use Ctrl+Shift+Tab to cycle through the documents in reverse order.
Another possibility is to use the arrow keys while keeping the Control key down, this way 
it is also possible to select any of the tool windows listed on the left side of the dialog.
If selecting any of the tool windows the same document will still be activated but focus is 
moved to the selected tool window.


![Overview plugin screenshot with light theme](https://github.com/leifmariposa/geany-ctrl-tab-plugin/blob/master/screenshots/screenshot.png?raw=true)

License
----------------

This plugin is distributed under the terms of the GNU General Public License
as published by the Free Software Foundation, either version 2 of the
License, or (at your option) any later version. You should have received a copy
of the GNU General Public License along with this plugin.  If not, see
<http://www.gnu.org/licenses/>. 

Contact
----------------

You can email me at &lt;leifmariposa(at)hotmail(dot)com&gt;
 
 
Bug reports and feature requests
----------------

To report a bug or ask for a new feature, please use the tracker
on GitHub: https://github.com/leifmariposa/geany-ctrl-tab-plugin/issues
