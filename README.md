 ## General information 
 A peer to peer file sharing application. The architecture of the peer to peer network used in the application is with super nodes. It was chosen in order to reduce the complexity of finding files, and in the same time keeping a low number of connected nodes to a single one, and also reducing the risk of a single point of failure. 
## Commands
add <filename> - share a new file <br />
search <filename> [condition] - search and download a file using its name <br />
shared - show the shared files from the current client <br />
show_debug_info - show some debug info(e.g the client is a super node, or a redundant super node)
