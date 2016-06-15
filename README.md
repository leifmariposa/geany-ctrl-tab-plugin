#Ctrl+Tab Plugin for Geany

<p class="topic-title first">
Contents
</p>
<ul class="simple">
<li>
<a class="reference internal" href="#about" id="id1" name="id1">About</a>
</li>
<li>
<a class="reference internal" href="#installation" id="id1" name="id1">Installation</a>
</li>
<li>
<a class="reference internal" href="#usage" id="id2" name="id2">Usage</a>
</li>
<li>
<a class="reference internal" href="#contact-developers" id="id4" name="id4">Contact developers</a>
</li>
</ul>
</div>
<div class="section" id="about">
<h3>
About
</h3>
<p>
Ctrl-Tab is a plugin for Geany that provides a dialog for rapid switching to last used document(s).
</p>
</div>
<div class="section" id="installation">
  
<h3>
Installation
</h3>

<p>Download the plugin from https://github.com/leifmariposa/geany-ctrl-tab-plugin
</p>
<p>
  
  
Then run the following commands:<br/>
  make<br/>
  sudo make install
</p>

</div>
<div class="section" id="usage">
<h3>
Usage
</h3>
<p>
After having enabled the plugin inside Geany through Geany's plugin manager,
you'll need to setup a keybinding for triggering the Ctrl-Tab dialog. Go to
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
moved to the selected tool window.<br>

<img src="https://github.com/leifmariposa/geany-ctrl-tab-plugin/blob/master/screenshots/screenshot.png" alt="Geany Ctrl+Tab Plugin" />
</p>

</div>
</div>
<div class="section" id="contact-developers">
<h3>
Contact developers
</h3>
<p>
Samy Badjoudj &lt;leifmariposa(at)hotmail(dot)com&gt;
</p>
</div>



