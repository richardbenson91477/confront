Confront
========

## Joystick or keyboard controlled app launcher

© 2020 Richard A. Benson (unless otherwise noted)

---

## Controls:  
  
left / right: change selection  
spacebar / enter / first button: launch application  
  
---
  
## Usage
  
confront [options]  
where [options] is zero or more of:
  * --full (enable fullscreen)
  * --border (enable window title/border)
  * --joy=n (specify joystick number, 0 for none)
  * --config=path (load alternate config file)
  * --help (this message)
  
---
## Config file format:
  
(NOTE: image/sound paths are relative to $PREFIX/share/confront/)
  
window x / y resolution
  
selection background image  
selection selected image  
selection launch image  
  
selection selected sound  
selection invalid sound  
selection launch sound  
  
app_1_path app_1_icon  
app_2_path app_2_icon  
...  
app_n_path app_n_icon  

---
## NOTES:

The example script "confront_poweroff" works with XFCE4: place it somewhere in your $PATH, e.g. "sudo install -o root -g root confront_poweroff /usr/local/bin/"  
  
For other desktops, best of luck
...
