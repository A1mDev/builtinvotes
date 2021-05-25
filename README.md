Last updates:
==============
1) Fix crash: server crash when unloading plugin or destroy vote handle during voting
2) Fix crash: if you unload the plugin during a vote and reload the map, then when the next vote is called, the server will crash. It seems SH_REMOVE_HOOK is not happening, and the function 'CL4DBaseBuiltinVote::OnClientCommand' returns the address of the old object, which has already been destroyed!
3) Add Natives: Game_IsVoteInProgress() and BuiltinVote_IsVoteInProgress(). 
4) Fix native IsBuiltinVoteInProgress(), also checks if the game voting is in progress at the moment. This blocks double voting without plugin updates (Block if the game vote is already in progress).

BuiltinVotes by Powerlord
==============
BuiltinVotes is a SourceMod Extension that lets plugins use the L4D/L4D2/TF2 built-in vote screens.

Build instructions
--------------
First clone the repository to your SourceMod SDK's public directory:

        cd sourcemod/public
        git clone https://github.com/A1mDev/builtinvotes.git

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
