# Snapshot Attack Demo

This is the demo of Unanticipated Snapshot Attack against Gramine.

### Build the demo

#### Install Intel SGX SDK and Occlum

First of all, anyone willing to run the demo must install Intel SGX SDK and Gramine (Please refer to https://gramine.readthedocs.io/en/latest/installation.html for more installation details.).

#### Build the target program

The entry point of the victim program is `gramine_run`, which is compiled from `gramine_run.c`. It first fetches a provisioned password, to generate a customized Redis configuration file, with the "requirepass" entry filled. Then it invokes a Redis server, according to the customized config file.

To build the config generation program, run `make SGX=1`. To build the Redis server, execute the following commands.

```
cd bash_redis
./download_and_build_redis_glibc.sh
```

### Run the demo

#### Make sure that the victim Redis program works

Open a terminal for the victim. Run `gramine-sgx redis-server`, checking if the Redis server works well.

Using command `redis-cli -h $Redis_Server_IP -p $Redis_Server_Port -a admin123456` to verify whether the password (admin123456) has been set in the config file.

Press `ctrl + C` to terminate the Redis server.

#### Snapshot capture

Prepare 2 terminals, one for the victim and one for the attacker. You can only open one more terminal (for the attacker) if one terminal has been opened already for the victim.

Run `gramine-sgx redis-server` at the victim terminal. This will start up the target program.

The attacker needs to take a snapshot (using `./take_snapshot_step.sh`) when he/she can determine that the snapshot is flushing. In the demo, the attacker can get a clear notice from the victim's terminal as follows.

```
...
Run ./take_snapshot NOW!
[DEBUG] line 4522: # passwords, then flags, or key patterns. However note that the additive
[DEBUG] line 4523: # and subtractive rules will CHANGE MEANING depending on the ordering.
[DEBUG] line 4524: # For instance see the following example:
...
```

Launch the `./take_snapshot_step.sh` script on the attacker's terminal.
 
For attacker's convenience, we reserve [time slots](https://github.com/potatoxz14/poc_gramine/blob/fa1b30e469442cb7f7f69255ed75635f375c7e5d/gramine_run.c#L40) to allow the attacker to run `./take_snapshot_step.sh`. In fact, the attacker who has host root privilege could intercept the enclave execution and capture any snapshots by modifying the untrusted part of SGX SDK.

#### Enclave replay

The attacker executes `replay_redis.sh` to replay the enclave using our collected snapshot. Any client can log onto the Redis server without authentication.

Example command for Redis client: 

```
redis-cli -h $Redis_Server_IP -p $Redis_Server_Port
keys *
```

If a message of `(error) NOAUTH Authentication required.` is prompted, it means you will need a password to access the Redis server. Type `auth $Your_Password` to pass the authentication. Use `auth admin123456` in this demo.

If a message of `(empty array)` is prompted, it means you do not need a password to access the Redis server.

GL & HF! 