<template>
	<v-form>

		<v-row>
			<v-col>

				<s-input
					type="text"
					data="config.project.entities[0].name"
					label="Name"
					hint="Name of the entity"
				></s-input>

			</v-col>
		</v-row>

		<v-row>
			<v-col>

				<s-input
					type="text"
					data="config.project.entities[0].entity.class"
					label="Class"
					hint="Class of the entity"
				></s-input>

			</v-col>
		</v-row>


		<v-row>
			<v-col>

				<div>Size</div>
				<div>Description</div>

			</v-col>
		</v-row>
		<v-row>
			<v-col>

				<s-input
					type="number"
					data="config.project.entities[0].entity.size.x"
					label="X"
				></s-input>

			</v-col>
			<v-col>

				<s-input
					type="number"
					data="config.project.entities[0].entity.size.y"
					label="Y"
				></s-input>

			</v-col>
			<v-col>

				<s-input
					type="number"
					data="config.project.entities[0].entity.size.z"
					label="Z"
				></s-input>

			</v-col>
		</v-row>

	</v-form>
</template>

<script>
	module.exports = {
	}
</script>

<style scoped>
</style>