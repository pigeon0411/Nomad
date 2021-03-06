/** THIS IS AN AUTOMATICALLY GENERATED FILE.
 *  DO NOT MODIFY BY HAND!!
 *
 *  Generated by zcm-gen
 **/

#include <zcm/zcm_coretypes.h>

#ifndef __sim_data_t_hpp__
#define __sim_data_t_hpp__



class sim_data_t
{
    public:
        int64_t    timestamp;

        int64_t    sequence_num;

        double     imu_orientation[4];

        double     imu_omega[3];

        double     imu_accel[3];

        double     q[12];

        double     q_dot[12];

        double     tau[12];

        double     com_orientation[4];

        double     com_pos[3];

        double     com_omega[3];

        double     com_vel[3];

    public:
        /**
         * Destructs a message properly if anything inherits from it
        */
        virtual ~sim_data_t() {}

        /**
         * Encode a message into binary form.
         *
         * @param buf The output buffer.
         * @param offset Encoding starts at thie byte offset into @p buf.
         * @param maxlen Maximum number of bytes to write.  This should generally be
         *  equal to getEncodedSize().
         * @return The number of bytes encoded, or <0 on error.
         */
        inline int encode(void* buf, uint32_t offset, uint32_t maxlen) const;

        /**
         * Check how many bytes are required to encode this message.
         */
        inline uint32_t getEncodedSize() const;

        /**
         * Decode a message from binary form into this instance.
         *
         * @param buf The buffer containing the encoded message.
         * @param offset The byte offset into @p buf where the encoded message starts.
         * @param maxlen The maximum number of bytes to reqad while decoding.
         * @return The number of bytes decoded, or <0 if an error occured.
         */
        inline int decode(const void* buf, uint32_t offset, uint32_t maxlen);

        /**
         * Retrieve the 64-bit fingerprint identifying the structure of the message.
         * Note that the fingerprint is the same for all instances of the same
         * message type, and is a fingerprint on the message type definition, not on
         * the message contents.
         */
        inline static int64_t getHash();

        /**
         * Returns "sim_data_t"
         */
        inline static const char* getTypeName();

        // ZCM support functions. Users should not call these
        inline int      _encodeNoHash(void* buf, uint32_t offset, uint32_t maxlen) const;
        inline uint32_t _getEncodedSizeNoHash() const;
        inline int      _decodeNoHash(const void* buf, uint32_t offset, uint32_t maxlen);
        inline static uint64_t _computeHash(const __zcm_hash_ptr* p);
};

int sim_data_t::encode(void* buf, uint32_t offset, uint32_t maxlen) const
{
    uint32_t pos = 0;
    int thislen;
    int64_t hash = (int64_t)getHash();

    thislen = __int64_t_encode_array(buf, offset + pos, maxlen - pos, &hash, 1);
    if(thislen < 0) return thislen; else pos += thislen;

    thislen = this->_encodeNoHash(buf, offset + pos, maxlen - pos);
    if (thislen < 0) return thislen; else pos += thislen;

    return pos;
}

int sim_data_t::decode(const void* buf, uint32_t offset, uint32_t maxlen)
{
    uint32_t pos = 0;
    int thislen;

    int64_t msg_hash;
    thislen = __int64_t_decode_array(buf, offset + pos, maxlen - pos, &msg_hash, 1);
    if (thislen < 0) return thislen; else pos += thislen;
    if (msg_hash != getHash()) return -1;

    thislen = this->_decodeNoHash(buf, offset + pos, maxlen - pos);
    if (thislen < 0) return thislen; else pos += thislen;

    return pos;
}

uint32_t sim_data_t::getEncodedSize() const
{
    return 8 + _getEncodedSizeNoHash();
}

int64_t sim_data_t::getHash()
{
    static int64_t hash = _computeHash(NULL);
    return hash;
}

const char* sim_data_t::getTypeName()
{
    return "sim_data_t";
}

int sim_data_t::_encodeNoHash(void* buf, uint32_t offset, uint32_t maxlen) const
{
    uint32_t pos = 0;
    int thislen;

    thislen = __int64_t_encode_array(buf, offset + pos, maxlen - pos, &this->timestamp, 1);
    if(thislen < 0) return thislen; else pos += thislen;

    thislen = __int64_t_encode_array(buf, offset + pos, maxlen - pos, &this->sequence_num, 1);
    if(thislen < 0) return thislen; else pos += thislen;

    thislen = __double_encode_array(buf, offset + pos, maxlen - pos, &this->imu_orientation[0], 4);
    if(thislen < 0) return thislen; else pos += thislen;

    thislen = __double_encode_array(buf, offset + pos, maxlen - pos, &this->imu_omega[0], 3);
    if(thislen < 0) return thislen; else pos += thislen;

    thislen = __double_encode_array(buf, offset + pos, maxlen - pos, &this->imu_accel[0], 3);
    if(thislen < 0) return thislen; else pos += thislen;

    thislen = __double_encode_array(buf, offset + pos, maxlen - pos, &this->q[0], 12);
    if(thislen < 0) return thislen; else pos += thislen;

    thislen = __double_encode_array(buf, offset + pos, maxlen - pos, &this->q_dot[0], 12);
    if(thislen < 0) return thislen; else pos += thislen;

    thislen = __double_encode_array(buf, offset + pos, maxlen - pos, &this->tau[0], 12);
    if(thislen < 0) return thislen; else pos += thislen;

    thislen = __double_encode_array(buf, offset + pos, maxlen - pos, &this->com_orientation[0], 4);
    if(thislen < 0) return thislen; else pos += thislen;

    thislen = __double_encode_array(buf, offset + pos, maxlen - pos, &this->com_pos[0], 3);
    if(thislen < 0) return thislen; else pos += thislen;

    thislen = __double_encode_array(buf, offset + pos, maxlen - pos, &this->com_omega[0], 3);
    if(thislen < 0) return thislen; else pos += thislen;

    thislen = __double_encode_array(buf, offset + pos, maxlen - pos, &this->com_vel[0], 3);
    if(thislen < 0) return thislen; else pos += thislen;

    return pos;
}

int sim_data_t::_decodeNoHash(const void* buf, uint32_t offset, uint32_t maxlen)
{
    uint32_t pos = 0;
    int thislen;

    thislen = __int64_t_decode_array(buf, offset + pos, maxlen - pos, &this->timestamp, 1);
    if(thislen < 0) return thislen; else pos += thislen;

    thislen = __int64_t_decode_array(buf, offset + pos, maxlen - pos, &this->sequence_num, 1);
    if(thislen < 0) return thislen; else pos += thislen;

    thislen = __double_decode_array(buf, offset + pos, maxlen - pos, &this->imu_orientation[0], 4);
    if(thislen < 0) return thislen; else pos += thislen;

    thislen = __double_decode_array(buf, offset + pos, maxlen - pos, &this->imu_omega[0], 3);
    if(thislen < 0) return thislen; else pos += thislen;

    thislen = __double_decode_array(buf, offset + pos, maxlen - pos, &this->imu_accel[0], 3);
    if(thislen < 0) return thislen; else pos += thislen;

    thislen = __double_decode_array(buf, offset + pos, maxlen - pos, &this->q[0], 12);
    if(thislen < 0) return thislen; else pos += thislen;

    thislen = __double_decode_array(buf, offset + pos, maxlen - pos, &this->q_dot[0], 12);
    if(thislen < 0) return thislen; else pos += thislen;

    thislen = __double_decode_array(buf, offset + pos, maxlen - pos, &this->tau[0], 12);
    if(thislen < 0) return thislen; else pos += thislen;

    thislen = __double_decode_array(buf, offset + pos, maxlen - pos, &this->com_orientation[0], 4);
    if(thislen < 0) return thislen; else pos += thislen;

    thislen = __double_decode_array(buf, offset + pos, maxlen - pos, &this->com_pos[0], 3);
    if(thislen < 0) return thislen; else pos += thislen;

    thislen = __double_decode_array(buf, offset + pos, maxlen - pos, &this->com_omega[0], 3);
    if(thislen < 0) return thislen; else pos += thislen;

    thislen = __double_decode_array(buf, offset + pos, maxlen - pos, &this->com_vel[0], 3);
    if(thislen < 0) return thislen; else pos += thislen;

    return pos;
}

uint32_t sim_data_t::_getEncodedSizeNoHash() const
{
    uint32_t enc_size = 0;
    enc_size += __int64_t_encoded_array_size(NULL, 1);
    enc_size += __int64_t_encoded_array_size(NULL, 1);
    enc_size += __double_encoded_array_size(NULL, 4);
    enc_size += __double_encoded_array_size(NULL, 3);
    enc_size += __double_encoded_array_size(NULL, 3);
    enc_size += __double_encoded_array_size(NULL, 12);
    enc_size += __double_encoded_array_size(NULL, 12);
    enc_size += __double_encoded_array_size(NULL, 12);
    enc_size += __double_encoded_array_size(NULL, 4);
    enc_size += __double_encoded_array_size(NULL, 3);
    enc_size += __double_encoded_array_size(NULL, 3);
    enc_size += __double_encoded_array_size(NULL, 3);
    return enc_size;
}

uint64_t sim_data_t::_computeHash(const __zcm_hash_ptr*)
{
    uint64_t hash = (uint64_t)0x32287bb6c6e34d45LL;
    return (hash<<1) + ((hash>>63)&1);
}

#endif
