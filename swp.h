/*
 * FROM THE BOOK - IGNORE ME!
 */

typedef u_char SwpSeqno;

typedef struct {
	SwpSeqno SeqNum; /* sequence number of this frame */
    SwpSeqno AckNum; /* ack of received frame */
	u_char Flags; /* up to 8 bits worth of flags */
} SwpHdr;


typedef struct {
	/* sender side state: */
	SwpSeqno LAR; /* seqno of last ACK received */
	SwpSeqno LFS; /* last frame sent */
	Semaphore sendWindowNotFull;
	SwpHdr hdr; /* pre-initialized header */
	
	struct sendQ_slot {
		Event timeout; /* event associated with send-timeout */
		Msg msg;
	} sendQ[SWS];
	
	/* receiver side state: */
	SwpSeqno NFE;
	/* seqno of next frame expected */
	struct recvQ_slot {
		int received;
		Msg msg; 
	} recvQ[RWS]; 
	/* is msg valid? */
	
} SwpState;
