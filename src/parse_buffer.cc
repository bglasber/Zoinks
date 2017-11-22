#include "parse_buffer.h"
#include <cassert>
#include <thread>
#include <chrono>
#include <iostream>

ParseBuffer::ParseBuffer() {
    ind = 0;
}

//You don't necessarily want to destroy all the lines in the buffer, since we may have
//put the line in the bin
void ParseBuffer::destroyBufferLines() {
    for( unsigned i = 0; i < ind; i++ ) {
        delete( parsed_lines[i] );
    }
}

bool ParseBuffer::addLine( std::vector<TokenWordPair> *line ) {
    assert( ind < LINES_IN_BUFFER );
    parsed_lines[ ind ] = line;
    ind++;
    return ind >= LINES_IN_BUFFER;
}

ParseBufferEngine::ParseBufferEngine() {
    term_when_out_of_buffers = false;
}

ParseBufferEngine::~ParseBufferEngine() {
    std::unique_lock<std::mutex> lk( mut );
    for( ParseBuffer *buff : ready_buffers ) {
        //Since these haven't been processed yet, we'll clean up the lines
        buff->destroyBufferLines();
        delete buff;
    }
    ready_buffers.clear();
}

void ParseBufferEngine::termWhenOutOfBuffers() {
    term_when_out_of_buffers = true;
    cv.notify_one();
}

ParseBuffer *ParseBufferEngine::getNextBuffer() {
    std::unique_lock<std::mutex> lk( mut );
    //Sleep until there are some ready buffers
    if( ready_buffers.empty() ) {
        if( term_when_out_of_buffers ) {
            return NULL;
        }
        cv.wait( lk, [ this ] { return !ready_buffers.empty() || term_when_out_of_buffers; } );
    }
    if( ready_buffers.empty() && term_when_out_of_buffers ) {
        return NULL;
    }
    ParseBuffer *buff = ready_buffers.front();
    ready_buffers.pop_front();
    return buff;
}

void ParseBufferEngine::putNextBuffer( ParseBuffer *buff ) {
RETRY:
    bool full = false;
    {
        std::unique_lock<std::mutex> lk( mut );
        if( ready_buffers.size() < 1000 ) {
            ready_buffers.push_back( buff );
            cv.notify_one();
        } else {
            full = true;
        }
    }
    //Back off
    if( full ) {
        std::this_thread::sleep_for( std::chrono::milliseconds( 300 ) );
        std::cout << "Backing off." << std::endl;
        goto RETRY;
    }
}


std::list<ParseBuffer *> &ParseBufferEngine::getReadyBuffers() {
    return ready_buffers;
}
