#include "stdafx.h"
#include "GameModel.h"

GameModel::GameModel(wstring materialFile, wstring meshFile)
	: velocity(0.0f, 0.0f, 0.0f)
{
	model = new Model();
	model->ReadMaterial(materialFile);
	model->ReadMesh(meshFile);

	shader = new Shader(Shaders + L"999_BoneAnimation.hlsl", "VS_Bone");
	shader2 = new Shader(Shaders + L"999_BoneAnimation.hlsl", "VS_ND", "PS_ND");
	shader3 = NULL;
	//shader3 = new Shader(Shaders + L"999_BoneAnimation.hlsl", "VS_Depth", "PS_Depth");
	for (Material* material : model->Materials())
		material->SetShader(shader);

	boneBuffer = new BoneBuffer();
	renderBuffer = new RenderBuffer();

	D3DXMatrixIdentity(&rotateMatrix);
}

GameModel::~GameModel()
{
	SAFE_DELETE(renderBuffer);
	SAFE_DELETE(boneBuffer);
	SAFE_DELETE(shader3);
	SAFE_DELETE(shader2);
	SAFE_DELETE(shader);
	SAFE_DELETE(model);
}

void GameModel::Velocity(D3DXVECTOR3 & vec)
{
	velocity = vec;
}

D3DXVECTOR3 GameModel::Velocity()
{
	return velocity;
}

D3DXVECTOR3 GameModel::CalcVelocity(D3DXVECTOR3& velocity)
{
	D3DXVECTOR3 v(0.0f, 0.0f, 0.0f);

	if (velocity.z != 0.0f)
		v += Direction() * velocity.z;
	if (velocity.x != 0.0f)
		v += Right() * velocity.x;
	if (velocity.y != 0.0f)
		v += Up() * velocity.y;

	return v;
}

void GameModel::AddPosition(D3DXVECTOR3 & vec)
{
	D3DXVECTOR3 pos = Position() + vec;
	Position(pos);
}

void GameModel::SetPosition(D3DXVECTOR3 & vec)
{
	Position(vec);
}

void GameModel::CalcPosition()
{
	if (D3DXVec3Length(&velocity) > 0.0f)
	{
		D3DXVECTOR3 v = CalcVelocity(velocity) * Time::Delta();

		AddPosition(v);
	}

}

void GameModel::Update()
{
	CalcPosition();

	D3DXMATRIX transform = Transformed();
	model->CopyAbsoluteBoneTo(transform, boneTransforms);
}

void GameModel::Render()
{
	if (Visible() == false) return;

	for (Material* material : model->Materials())
		material->SetShader(shader);
	
	boneBuffer->SetBones(&boneTransforms[0], boneTransforms.size());
	boneBuffer->SetVSBuffer(2);

	for (ModelMesh* mesh : model->Meshes())
	{
		int index = mesh->ParentBoneIndex();
		
		renderBuffer->Data.BoneNumber = index;
		renderBuffer->SetVSBuffer(3);

		mesh->Render();
	}
}

void GameModel::PreRender()
{
	if (Visible() == false) return;

	for (Material* material : model->Materials())
		material->SetShader(shader2);

	boneBuffer->SetBones(&boneTransforms[0], boneTransforms.size());
	boneBuffer->SetVSBuffer(2);


	for (ModelMesh* mesh : model->Meshes())
	{
		int index = mesh->ParentBoneIndex();

		renderBuffer->Data.BoneNumber = index;
		renderBuffer->SetVSBuffer(3);

		mesh->Render();
	}
}

void GameModel::PreRender2()
{
	if (Visible() == false) return;

	//for (Material* material : model->Materials())
	//	material->SetShader(shader3);
	//
	//boneBuffer->SetBones(&boneTransforms[0], boneTransforms.size());
	//boneBuffer->SetVSBuffer(2);
	//
	//for (ModelMesh* mesh : model->Meshes())
	//{
	//	int index = mesh->ParentBoneIndex();
	//
	//	renderBuffer->Data.BoneNumber = index;
	//	renderBuffer->SetVSBuffer(3);
	//
	//	mesh->Render();
	//}
}

void GameModel::ImGuiRender()
{
}

void GameModel::Rotate(D3DXVECTOR2 amount)
{
	amount *= Time::Delta();

	/*
	mouse 좌우를 카메라의 y축 회전으로 했었던 것과 유사함
	*/

	D3DXMATRIX axis;
	D3DXMatrixRotationAxis(&axis, &Right(), amount.y);

	D3DXMATRIX Y;
	D3DXMatrixRotationY(&Y, Math::ToRadian(amount.x));
 
	rotateMatrix = axis * Y;

	D3DXMATRIX world = World();
	World(rotateMatrix * world);
}
