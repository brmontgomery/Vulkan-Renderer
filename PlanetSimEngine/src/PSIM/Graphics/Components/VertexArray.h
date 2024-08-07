#pragma once

#include <memory>
#include "PSIM/Graphics/Components/Buffer.h"

class VertexArray
{
public:
	virtual ~VertexArray() {}

	virtual void Bind() const = 0;
	virtual void Unbind() const = 0;

	virtual void AddVertexBuffer(const Ref<VertexBuffer>& vertexBuffer) = 0;
	virtual void SetIndexBuffer(const Ref<IndexBuffer>& indexBuffer) = 0;

	virtual const std::vector<Ref<VertexBuffer>>& GetVertexBuffers() const = 0;
	virtual const Ref<IndexBuffer>& GetIndexBuffer() const = 0;

	virtual void* getBindingDescription() = 0;
	virtual void* getAttributeDescriptions() = 0;

	virtual void* getVertexBuffersBuffers() = 0;
	virtual void* getIndexBufferBuffer() = 0;

	virtual void cleanUp() = 0;

	static Ref<VertexArray> Create();
};