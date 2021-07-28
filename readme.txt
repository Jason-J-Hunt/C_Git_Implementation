Server sits and listens for connections upon getting a connection it spawns a new thread and hands the connection to the thread to
be handled. Upon being sent to the handler it right away attempts to lock a mutex and if it cant it sits and wait until it can.
This ensure multiple clients arent attempting to modify the same things at the same time.