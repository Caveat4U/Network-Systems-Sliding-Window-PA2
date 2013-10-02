
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

static int
sendSWP(SwpState *state, Msg *frame)
{
	struct sendQ_slot *slot;
	hbuf[HLEN];
	/* wait for send window to open */
	semWait(&state->sendWindowNotFull);
	state->hdr.SeqNum = ++state->LFS;
	slot = &state->sendQ[state->hdr.SeqNum % SWS];
	store_swp_hdr(state->hdr, hbuf);
	msgAddHdr(frame, hbuf, HLEN);
	msgSaveCopy(&slot->msg, frame);
	slot->timeout = evSchedule(swpTimeout, slot, SWP_SEND_TIMEOUT);
	return send(LINK, frame);
}

static int
deliverSWP(SwpState state, Msg *frame)
{
	SwpHdr hdr;
	char *hbuf;
	hbuf = msgStripHdr(frame, HLEN);
	load_swp_hdr(&hdr, hbuf)
	if (hdr->Flags & FLAG_ACK_VALID)
	{
		/* received an acknowledgment---do SENDER side */
		if (swpInWindow(hdr.AckNum, state->LAR + 1, state->LFS))
		{
			do
			{
				struct sendQ_slot *slot;
				slot = &state->sendQ[++state->LAR % SWS];
				evCancel(slot->timeout);
				msgDestroy(&slot->msg);
				semSignal(&state->sendWindowNotFull);
			} while (state->LAR != hdr.AckNum);
		}
	}
	if (hdr.Flags & FLAG_HAS_DATA)
	{
		struct recvQ_slot *slot;
		/* received data packet---do RECEIVER side */
		slot = &state->recvQ[hdr.SeqNum % RWS];
		if (!swpInWindow(hdr.SeqNum, state->NFE, state->NFE + RWS - 1))
		{
			/* drop the message */
			return SUCCESS;
		}
		msgSaveCopy(&slot->msg, frame);
		slot->received = TRUE;
		if (hdr.SeqNum == state->NFE)
		{
			Msg m;
			while (slot->received)
			{
				deliver(HLP, &slot->msg);
				msgDestroy(&slot->msg);
				slot->received = FALSE;
				slot = &state->recvQ[++state->NFE % RWS];
			}
			/* send ACK: */
			prepare_ack(&m, state->NFE - 1);
			send(LINK, &m);
			msgDestroy(&m);
		}
	}
	return SUCCESS;
}

static bool
swpInWindow(SwpSeqno seqno, SwpSeqno min, SwpSeqno max)
{
	SwpSeqno pos, maxpos;
	pos = seqno - min;
	maxpos = max - min + 1;
	/* pos *should* be in range [0..MAX) */
	/* maxpos is in range [0..MAX] */
	return pos < maxpos;
}
