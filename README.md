# Build

```
mkdir build && cd build
cmake .. -DPSI_PLUGIN_INCLUDE=../psi/src/plugins/include
make
```

You will need Psi includes for exactly your Psi version. For example, Psi 1.5 includes  can be downloaded (to make aforementioned ../psi/src/plugins/include directory appear in the tree) from the plugin project's root via:
```
git clone --depth 1 --branch 1.5 https://github.com/psi-im/psi.git
```

Install liblowernah.so into Psi’s plugin directory and enable LowerNah in settings.

# Config

Put one file per room in Psi’s config directory, named after the room JID:

```
<configdir>/lowernah/room@conference.example.com
```

For Linux it's usually
```
~/.config/psi/lowernah/room@conference.example.com
```

In the config file place one nickname or occupant JID per line (# comments allowed). The directory is created on enable and watched with QFileSystemWatcher so edits apply without restarting Psi.

