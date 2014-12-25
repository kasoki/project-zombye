#include <zombye/rendering/vertex_array.hpp>

namespace zombye {
    vertex_array::vertex_array() noexcept {
        glGenVertexArrays(1, &id_);
        bind();
    }

    vertex_array::~vertex_array() noexcept {
        glDeleteVertexArrays(1, &id_);
    }

    void vertex_array::bind() const noexcept {
        glBindVertexArray(id_);
    }

    void vertex_array::bind_index_buffer(const index_buffer& buffer) {
        bind();
        buffer.bind();
    }

    void vertex_array::bind_vertex_attribute(const vertex_buffer& buffer, uint32_t index, int32_t size, GLenum type,
    bool normalized, size_t stride, intptr_t offset) noexcept {
        buffer.bind();
        glEnableVertexAttribArray(index);
        glVertexAttribPointer(index, size, type, normalized, stride, reinterpret_cast<GLvoid*>(offset));
    }
}