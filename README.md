Fix server crash
==============
1) Fix: server crash when unloading plugin or destroy vote handle during voting
2) Fix: if you unload the plugin during a vote and reload the map, then when the next vote is called, the server will crash. It seems SH_REMOVE_HOOK is not happening, and the function 'CL4DBaseBuiltinVote::OnClientCommand' returns the address of the old object, which has already been destroyed!

BuiltinVotes by Powerlord
==============
BuiltinVotes is a SourceMod Extension that lets plugins use the L4D/L4D2/TF2 built-in vote screens.

Build instructions
--------------
First clone the repository to your SourceMod SDK's public directory:

        cd sourcemod/public
        git clone https://github.com/mvandorp/builtinvotes.git

Then run *make* to build the project:

        cd builtinvotes
        make ENGINE="left4dead2"

This will compile the project to create a sourcemod extension.

Install
--------------
Download one of the binaries archives from the releases on the GitHub repository and simply extract the archive to your server's 'left4dead2' directory to install it.

Credits
--------------
BuiltinVotes is created by "Powerlord"
https://forums.alliedmods.net/showthread.php?t=162164
https://code.google.com/p/sourcemod-builtinvotes/

This entire project is released under the AlliedModders modified GPLv3.
