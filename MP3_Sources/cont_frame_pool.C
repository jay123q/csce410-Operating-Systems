/*
 File: ContFramePool.C

 Author:
 Date  :

 */

/*--------------------------------------------------------------------------*/
/*
 POSSIBLE IMPLEMENTATION
 -----------------------

 The class SimpleFramePool in file "simple_frame_pool.H/C" describes an
 incomplete vanilla implementation of a frame pool that allocates
 *single* frames at a time. Because it does allocate one frame at a time,
 it does not guarantee that a sequence of frames is allocated contiguously.
 This can cause problems.

 The class ContFramePool has the ability to allocate either single frames,
 or sequences of contiguous frames. This affects how we manage the
 free frames. In SimpleFramePool it is sufficient to maintain the free
 frames.
 In ContFramePool we need to maintain free *sequences* of frames.

 This can be done in many ways, ranging from extensions to bitmaps to
 free-lists of frames etc.

 IMPLEMENTATION:

 One simple way to manage sequences of free frames is to add a minor
 extension to the bitmap idea of SimpleFramePool: Instead of maintaining
 whether a frame is FREE or ALLOCATED, which requires one bit per frame,
 we maintain whether the frame is FREE, or ALLOCATED, or HEAD-OF-SEQUENCE.
 The meaning of FREE is the same as in SimpleFramePool.
 If a frame is marked as HEAD-OF-SEQUENCE, this means that it is allocated
 and that it is the first such frame in a sequence of frames. Allocated
 frames that are not first in a sequence are marked as ALLOCATED.

 NOTE: If we use this scheme to allocate only single frames, then all
 frames are marked as either FREE or HEAD-OF-SEQUENCE.

 NOTE: In SimpleFramePool we needed only one bit to store the state of
 each frame. Now we need two bits. In a first implementation you can choose
 to use one char per frame. This will allow you to check for a given status
 without having to do bit manipulations. Once you get this to work,
 revisit the implementation and change it to using two bits. You will get
 an efficiency penalty if you use one char (i.e., 8 bits) per frame when
 two bits do the trick.

 DETAILED IMPLEMENTATION:

 How can we use the HEAD-OF-SEQUENCE state to implement a contiguous
 allocator? Let's look a the individual functions:

 Constructor: Initialize all frames to FREE, except for any frames that you
 need for the management of the frame pool, if any.

 get_frames(_n_frames): Traverse the "bitmap" of states and look for a
 sequence of at least _n_frames entries that are FREE. If you find one,
 mark the first one as HEAD-OF-SEQUENCE and the remaining _n_frames-1 as
 ALLOCATED.

 release_frames(_first_frame_no): Check whether the first frame is marked as
 HEAD-OF-SEQUENCE. If not, something went wrong. If it is, mark it as FREE.
 Traverse the subsequent frames until you reach one that is FREE or
 HEAD-OF-SEQUENCE. Until then, mark the frames that you traverse as FREE.

 mark_inaccessible(_base_frame_no, _n_frames): This is no different than
 get_frames, without having to search for the free sequence. You tell the
 allocator exactly which frame to mark as HEAD-OF-SEQUENCE and how many
 frames after that to mark as ALLOCATED.

 needed_info_frames(_n_frames): This depends on how many bits you need
 to store the state of each frame. If you use a char to represent the state
 of a frame, then you need one info frame for each FRAME_SIZE frames.

 A WORD ABOUT RELEASE_FRAMES():

 When we releae a frame, we only know its frame number. At the time
 of a frame's release, we don't know necessarily which pool it came
 from. Therefore, the function "release_frame" is static, i.e.,
 not associated with a particular frame pool.

 This problem is related to the lack of a so-called "placement delete" in
 C++. For a discussion of this see Stroustrup's FAQ:
 http://www.stroustrup.com/bs_faq2.html#placement-delete

 */
/*--------------------------------------------------------------------------*/

/*--------------------------------------------------------------------------*/
/* DEFINES */
/*--------------------------------------------------------------------------*/

/* -- (none) -- */

/*--------------------------------------------------------------------------*/
/* INCLUDES */
/*--------------------------------------------------------------------------*/

#include "cont_frame_pool.H"
#include "console.H"
#include "utils.H"
#include "assert.H"

/*--------------------------------------------------------------------------*/
/* DATA STRUCTURES */
/*--------------------------------------------------------------------------*/

ContFramePool *ContFramePool::head_frame = NULL;
ContFramePool *ContFramePool::tail_frame = NULL;

/*--------------------------------------------------------------------------*/
/* CONSTANTS */
/*--------------------------------------------------------------------------*/

/* -- (none) -- */

/*--------------------------------------------------------------------------*/
/* FORWARDS */
/*--------------------------------------------------------------------------*/

/* -- (none) -- */

/*--------------------------------------------------------------------------*/
/* METHODS FOR CLASS   C o n t F r a m e P o o l */
/*--------------------------------------------------------------------------*/

ContFramePool::FrameState ContFramePool::get_state(unsigned long _frame_no) {
    unsigned int bitmap_index = _frame_no / 8;
    unsigned char mask = 0xFF << (_frame_no % 8);
     unsigned char hosMask = 0xFF >> (_frame_no % 4) *2;
    if( bitmap[bitmap_index] |=  hosMask)
    {
        return FrameState::HoS;
    }
    return ((bitmap[bitmap_index] & mask) == 0) ? FrameState::Used : FrameState::Free;
    
}
void ContFramePool::set_state(unsigned long _frame_no, FrameState _state)
{
    unsigned int bitmap_index = _frame_no / 8;
    unsigned char mask = 0xFF << (_frame_no % 8);
    unsigned char hosMask = 0xFF << (_frame_no % 4) *2;

    switch (_state)
    {
    case FrameState::Used:
        bitmap[bitmap_index] ^= mask;
        n_free_frames--;
        break;
    case FrameState::Free:
        bitmap[bitmap_index] |= mask;
        n_free_frames++;
        break;
    case FrameState::HoS:
        bitmap[bitmap_index] |= hosMask;
        n_free_frames--;
        break;
    }
}
ContFramePool::ContFramePool(unsigned long _base_frame_no,
                             unsigned long _n_frames,
                             unsigned long _info_frame_no)
{
    // The first thing we check is does our frames pass our machine size
    assert(_n_frames <= FRAME_SIZE * 8);
    base_frame_no = _base_frame_no;
    n_frames = _n_frames;
    info_frame_no = _info_frame_no;

    if (info_frame_no == 0)
    {
        bitmap = (unsigned char *)(base_frame_no * FRAME_SIZE);
    }
    else
    {
        bitmap = (unsigned char *)(info_frame_no * FRAME_SIZE);
    }

    assert((n_frames % 8) == 0)
    for (int i = 0; i < _n_frames; i++)
    {;
        set_state(i, FrameState::Free);
    }

    if (_info_frame_no == 0)
    {
        set_state(0, FrameState::Used);

    }
    // // rev up our linked list of frames
    if (head_frame == NULL)
    {
        head_frame = this;
        tail_frame = this;
        head_frame->next_frame = nullptr;
        tail_frame->prev_frame = nullptr;
    }
    else
    {
        auto old_tail = tail_frame;
        tail_frame = this;
        old_tail->next_frame = tail_frame;
        tail_frame->prev_frame = old_tail;
    }

    Console::puts("Frame Pool initialized\n");

    // TODO: IMPLEMENTATION NEEEDED!
    // Console::puts("ContframePool::Constructor not implemented!\n");
    // assert(false);
}




unsigned long ContFramePool::get_frames(unsigned int _n_frames)
{
    unsigned int needed_frames = _n_frames;


    if (_n_frames >= n_free_frames)
    {
        Console::puts("PANIC NO MORE FRAMES AHHHHHHHHHHHHHHHHHHHHHHHHH LINE 232 ");
        assert(false);
    }
    int index = 0;
    // count allocated frames
    while (get_state(index) == FrameState::Used)
    {
        index++;
    }
    // where are we going to from the allocated frames
    // if ( get_state(index) == FrameState::HoS)
    // {
    //     set_state(index, FrameState::Used);
    // }
    // else
    // {
    //     set_state(index, FrameState::HoS);
    // }
    
    set_state(index, FrameState::Used);
    return (base_frame_no + index);

    // TODO: IMPLEMENTATION NEEEDED!
}

void ContFramePool::mark_inaccessible(unsigned long _base_frame_no,
                                      unsigned long _n_frames)
{
    // TODO: IMPLEMENTATION NEEEDED!
    // Mark all frames in the range as being used.
    // set_state(base_frame_no , FrameState::HoS );
    for (unsigned long index = _base_frame_no + 1; index < _base_frame_no + _n_frames; index++)
    {
        set_state(index - base_frame_no , FrameState::Used);
    }

}
void ContFramePool::release_a_single_frame(unsigned long _first_frame_no)
{
    // find where we are first frame from the base
    unsigned int frame_position = _first_frame_no - base_frame_no;
    set_state( frame_position , FrameState::Free);


}

void ContFramePool::release_frames(unsigned long _first_frame_no)
{
    // TODO: IMPLEMENTATION NEEEDED!
    head_frame->release_a_single_frame(_first_frame_no);
}

unsigned long ContFramePool::needed_info_frames(unsigned long _n_frames)
{
    // TODO: IMPLEMENTATION NEEEDED!
    unsigned long carry_frames = (_n_frames * 8) % (4096 * 8);
    if (carry_frames > 0)
    {
        carry_frames = 1;
    }
    else
    {
        carry_frames = 0;
    }
    unsigned int needed_info_frames = (_n_frames * 8) / (4096 * 8) + carry_frames;
    return needed_info_frames;
}
